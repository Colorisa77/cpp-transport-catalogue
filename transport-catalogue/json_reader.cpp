#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <sstream>
using namespace std::literals;

namespace json_reader {
    void FillStopsByRequestBody(transport_catalogue::TransportCatalogue& transport_catalogue, const json::Node& request_body) {
        geo::Coordinates coordinates {
            request_body.AsDict().at("latitude"s).AsDouble(), 
            request_body.AsDict().at("longitude"s).AsDouble()
        };
        transport_catalogue.AddStop(request_body.AsDict().at("name"s).AsString(), coordinates);
    }

    void FillStopToStopDistances(transport_catalogue::TransportCatalogue& transport_catalogue, const json::Node& stop_to_stop_distance) {
        for(const auto& [second_stop_name, distance] : stop_to_stop_distance.AsDict().at("road_distances"s).AsDict()) {
            transport_catalogue.SetStopToStopDistances(
                stop_to_stop_distance.AsDict().at("name"s).AsString(), 
                second_stop_name, 
                distance.AsDouble()
            );
        }
    }

    void FillRoutesByRequestBody(transport_catalogue::TransportCatalogue& transport_catalogue, const json::Node& add_buses_request) {
        std::vector<std::string> stops;
        for(const auto& stop : add_buses_request.AsDict().at("stops"s).AsArray()) {
            stops.push_back(stop.AsString());
        }
        transport_catalogue.AddBus(
            add_buses_request.AsDict().at("name"s).AsString(),
            stops, 
            add_buses_request.AsDict().at("is_roundtrip"s).AsBool()
        );
    }

    std::string EscapingAString(const std::ostringstream& svg_output) {
    std::ostringstream escaped_output;
    for (char ch : svg_output.str()) {
            switch (ch) {
                case '"':
                    escaped_output << '\"';
                    break;
                default:
                    escaped_output << ch;
                    break;
            }
        }
        return escaped_output.str();
    }   

    json::Node AddSvgOutput(const std::ostringstream& svg_output, const json::Node& request_body) {
        json::Node response = json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(request_body.AsDict().at("id"s).AsInt())
                .Key("map"s).Value(EscapingAString(svg_output))
            .EndDict()
        .Build().AsDict();
        return response;
    }


    json::Node GenerateResponse(const request_handler::RequestHandler& request_handler, const json::Node& request_body, const std::ostringstream& svg_output) {
        json::Node result;
        if(request_body.AsDict().at("type"s).AsString() == "Stop"s) {
            result = AddStopInfoResponse(request_handler, request_body);
        }
        if(request_body.AsDict().at("type"s).AsString() == "Bus"s) {
            result = AddBusInfoResponse(request_handler, request_body);
        }
        if(request_body.AsDict().at("type"s).AsString() == "Route"s) {
            result = AddRouteInfoResponse(request_handler, request_body);
        }
        if(request_body.AsDict().at("type"s).AsString() == "Map"s) {
            result = AddSvgOutput(svg_output, request_body);
        }
        return result;
    }

    json::Node AddStopInfoResponse(const request_handler::RequestHandler& request_handler, const json::Node& request_body) {
        std::optional<transport_catalogue::StopInfo> stop_info = request_handler.GetStopInfo(request_body.AsDict().at("name"s).AsString());
        if(stop_info && stop_info.value().is_empty == false) {
            json::Array stop_buses;
            for(const auto& bus : stop_info.value().stop_buses) {
                std::string string_result = {bus.begin(), bus.end()};
                stop_buses.push_back(string_result);
            }
            json::Dict response = json::Builder{}
                .StartDict()
                    .Key("buses"s).Value(stop_buses)
                    .Key("request_id"s).Value(request_body.AsDict().at("id"s).AsInt())
                .EndDict()
            .Build().AsDict();
            return response;
        }
        json::Dict response = json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(request_body.AsDict().at("id"s).AsInt())
                .Key("error_message"s).Value("not found"s)
            .EndDict()
        .Build().AsDict();
        return response;
    }

    json::Node AddBusInfoResponse(const request_handler::RequestHandler& request_handler, const json::Node& request_body) {
        std::optional<transport_catalogue::BusInfo> bus_info = request_handler.GetBusInfo(request_body.AsDict().at("name"s).AsString());
        if(bus_info && bus_info.value().is_empty == false) {
            json::Dict response = json::Builder{}
                .StartDict()
                    .Key("curvature"s).Value(bus_info.value().curvature)
                    .Key("request_id"s).Value(request_body.AsDict().at("id"s).AsInt())
                    .Key("route_length"s).Value(bus_info.value().route_length)
                    .Key("stop_count"s).Value(static_cast<int>(bus_info.value().count_all_stops))
                    .Key("unique_stop_count"s).Value(static_cast<int>(bus_info.value().count_unique_stops))
                .EndDict()
            .Build().AsDict();
            return response;
        }
        json::Dict response = json::Builder{}
            .StartDict()
                .Key("request_id"s)
                .Value(request_body.AsDict().at("id"s).AsInt())
                .Key("error_message"s).Value("not found"s)
            .EndDict()
        .Build().AsDict();
        return response;
    }

    json::Node AddRouteInfoResponse(const request_handler::RequestHandler& request_handler, const json::Node& request_body) {
        std::optional<transport_catalogue::RouteInfo> route_info = request_handler.GetRouteInfo(request_body.AsDict().at("from"s).AsString(), request_body.AsDict().at("to"s).AsString());
        if(route_info) {
            json::Dict result;
            json::Builder response;
            response.StartDict()
                    .Key("request_id"s).Value(request_body.AsDict().at("id"s).AsInt())
                    .Key("total_time"s).Value(route_info.value().total_time)
                    .Key("items"s).StartArray();
                
            for(const auto& route : route_info.value().list_of_items) {
                response.StartDict();
                if(route.type == "Wait"s) {
                    response.Key("type"s).Value(route.type)
                            .Key("stop_name"s).Value((std::string)route.bus_or_stop_name)
                            .Key("time"s).Value(route.time);
                } else {
                    response.Key("type"s).Value(route.type)
                            .Key("bus"s).Value((std::string)route.bus_or_stop_name)
                            .Key("span_count"s).Value(route.span.value())
                            .Key("time"s).Value(route.time);
                }
                response.EndDict();
            }
            response.EndArray().EndDict();
            result = response.Build().AsDict();
            return result;
        }
        json::Dict response = json::Builder{}
            .StartDict()
                .Key("request_id"s)
                .Value(request_body.AsDict().at("id"s).AsInt())
                .Key("error_message"s).Value("not found"s)
            .EndDict()
        .Build().AsDict();
        return response;     
    }


    JsonReader::JsonReader(std::istream& input) 
        : doc_(json::Load(input)) {
    }

    const json::Dict& JsonReader::GetSerializationSettingsRequests() const {
        return doc_.GetRoot().AsDict().at("serialization_settings"s).AsDict();
    }

    const json::Array& JsonReader::GetBaseRequests() const {
        return doc_.GetRoot().AsDict().at("base_requests"s).AsArray();
    }

    const json::Dict& JsonReader::GetRenderSettings() const {
        return doc_.GetRoot().AsDict().at("render_settings"s).AsDict();
    }

    const json::Dict& JsonReader::GetRouteSettings() const {
        return doc_.GetRoot().AsDict().at("routing_settings"s).AsDict();
    }

    const json::Array& JsonReader::GetStatRequests() const {
        return doc_.GetRoot().AsDict().at("stat_requests"s).AsArray();
    }

    void JsonReader::AddRouteCoordinates(geo::Coordinates coordinates) {
        route_coordinates_.push_back(coordinates);
    }

    const std::vector<geo::Coordinates>& JsonReader::GetRoutesCoordinate() const {
        return route_coordinates_;
    }

    void JsonReader::AddStopToStopRequest(const json::Node& node) {
        requests_.stop_requests.push_back(node);
    }

    void JsonReader::AddBusRequest(const json::Node& node) {
        requests_.bus_requests.push_back(node);
    }

    const Requests& JsonReader::GetRequests() const {
        return requests_;
    }

    void JsonReader::AddBusName(const std::string& bus_name) {
        buses_names_.insert(bus_name);
    }

    const std::set<std::string>& JsonReader::GetBusNames() const {
        return buses_names_;
    }

    void FillingTransportCatalogue(transport_catalogue::TransportCatalogue& transport_catalogue, JsonReader& json_reader) {

        for(const auto& request_body : json_reader.GetBaseRequests()) {
            if(request_body.AsDict().at("type"s).AsString() == "Stop"s) {
                FillStopsByRequestBody(transport_catalogue, request_body);
                json::Node stop_to_stop_distance_request = json::Dict{
                        {"name"s, request_body.AsDict().at("name"s).AsString()},
                        {"road_distances"s, request_body.AsDict().at("road_distances"s).AsDict()}
                };
                json_reader.AddStopToStopRequest(stop_to_stop_distance_request);
                json_reader.AddRouteCoordinates({
                                                        request_body.AsDict().at("latitude"s).AsDouble(),
                                                        request_body.AsDict().at("longitude"s).AsDouble()
                                                });
                continue;
            }
            if(request_body.AsDict().at("type"s).AsString() == "Bus"s) {
                json_reader.AddBusRequest(request_body);
            }
        }
        for(const auto& add_stop_to_stop_distance_request : json_reader.GetRequests().stop_requests) {
            FillStopToStopDistances(transport_catalogue, add_stop_to_stop_distance_request);
        }

        for(const auto& add_buses_request : json_reader.GetRequests().bus_requests) {
            FillRoutesByRequestBody(transport_catalogue, add_buses_request);
            json_reader.AddBusName(add_buses_request.AsDict().at("name"s).AsString());

        }
    }

    renderer::RenderSettings SetRenderSettings(const json::Node& render_settings) {
        renderer::RenderSettings result;
        result.width = render_settings.AsDict().at("width"s).AsDouble();
        result.height = render_settings.AsDict().at("height"s).AsDouble();
        result.padding = render_settings.AsDict().at("padding"s).AsDouble();
        result.line_width = render_settings.AsDict().at("line_width"s).AsDouble();
        result.stop_radius = render_settings.AsDict().at("stop_radius"s).AsDouble();
        result.bus_label_font_size = render_settings.AsDict().at("bus_label_font_size"s).AsInt();

        svg::Point point_bus_label_offset{render_settings.AsDict().at("bus_label_offset"s).AsArray().at(0).AsDouble(), render_settings.AsDict().at("bus_label_offset"s).AsArray().at(1).AsDouble()};
        result.bus_label_offset = point_bus_label_offset;

        result.stop_label_font_size = render_settings.AsDict().at("stop_label_font_size"s).AsInt();

        svg::Point point_stop_label_offset{render_settings.AsDict().at("stop_label_offset"s).AsArray().at(0).AsDouble(), render_settings.AsDict().at("stop_label_offset"s).AsArray().at(1).AsDouble()};
        result.stop_label_offset = point_stop_label_offset;

        svg::Color underlayer_color (renderer::GetColorFromUnderlayerColorNode(render_settings));
        result.underlayer_color = underlayer_color;

        result.underlayer_width = render_settings.AsDict().at("underlayer_width"s).AsDouble();

        result.color_palette = renderer::GetColorFromColorPaletteNode(render_settings);

        return result;
    }

    graph::RouteSettings SetRoutingSettings(const json::Node& routing_settings) {
        graph::RouteSettings result;

        result.bus_velocity = routing_settings.AsDict().at("bus_velocity"s).AsDouble();
        result.bus_wait_time = routing_settings.AsDict().at("bus_wait_time"s).AsInt();

        return result;
    }

    void SequentialRequestProcessing(
        transport_catalogue::TransportCatalogue& transport_catalogue,
        renderer::RenderSettings& render_settings,
        graph::RouteSettings& route_settings,
        router::TransportRouter& router, 
        renderer::MapRenderer& map_render, 
        JsonReader& json_reader,
        std::ostream& output, 
        request_handler::RequestHandler& request_handler) {

        router.FillTransportRouter(transport_catalogue, route_settings.bus_velocity, route_settings.bus_wait_time);

        renderer::MapVisualizationSettings settings(
            render_settings.width,
            render_settings.height,
            render_settings.padding
        );
        std::vector<geo::Coordinates> route_coordinates = request_handler.GetCoordinatesFromStopsWithCoordinates();
        renderer::SphereProjector projector(
            route_coordinates.begin(), 
            route_coordinates.end(), 
            settings.max_width, 
            settings.max_height, 
            settings.padding
        );
        map_render.SetPossibleColors(render_settings.color_palette);

        std::vector<std::string_view> bus_names = request_handler.GetAllBusesFromCatalogue();
        std::sort(bus_names.begin(), bus_names.end());

        for(const auto& route_name : bus_names) {
            const auto& bus = request_handler.GetBus(route_name);
            if(bus != nullptr) {
                std::string bus_name{route_name};
                size_t pos = (bus->stops.size()) / 2;
                map_render.AddNewPointByRouteName(bus_name, projector(bus->stops.front()->coordinates), render_settings);
                map_render.AddNewTextForRoute(bus_name, projector(bus->stops.front()->coordinates), render_settings);
                map_render.AddNewCircleForStop(bus->stops.front()->name, projector(bus->stops.front()->coordinates), render_settings);
                map_render.AddNewTextForStop(bus->stops.front()->name, projector(bus->stops.front()->coordinates), render_settings);
                if(bus->is_circle == false && bus->stops.at(pos) != bus->stops.front()) {
                    map_render.AddNewTextForRoute(bus_name, projector(bus->stops.at(pos)->coordinates), render_settings);
                }
                for(size_t i = 1; i < bus->stops.size() - 1; ++i) {
                    map_render.AddNewPointByRouteName(bus_name, projector(bus->stops.at(i)->coordinates), render_settings);
                    map_render.AddNewCircleForStop(bus->stops.at(i)->name, projector(bus->stops.at(i)->coordinates), render_settings);
                    map_render.AddNewTextForStop(bus->stops.at(i)->name, projector(bus->stops.at(i)->coordinates), render_settings);
                }
                map_render.AddNewPointByRouteName(bus_name, projector(bus->stops.back()->coordinates), render_settings);
                map_render.ChangeCurrentColor();
            }
        }

        std::ostringstream svg_output;
        request_handler.RenderMap(svg_output);

        json::Array result;
        json::Builder responses;
        responses.StartArray();
        for(const auto& request_body : json_reader.GetStatRequests()) {
            json::Node value = GenerateResponse(request_handler, request_body, svg_output);
            responses.Value(value);
        }
        responses.EndArray();
        result = responses.Build().AsArray();
        json::Print(json::Document{result}, output);

    }
}