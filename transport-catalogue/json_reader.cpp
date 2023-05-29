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


    json::Node GenerateResponse(const request_handler::RequestHandler& request_handler, const router::TransportRouter<double>& router, const json::Node& request_body, const std::ostringstream& svg_output) {
        json::Node result;
        if(request_body.AsDict().at("type"s).AsString() == "Stop"s) {
            result = AddStopInfoResponse(request_handler, request_body);
        }
        if(request_body.AsDict().at("type"s).AsString() == "Bus"s) {
            result = AddBusInfoResponse(request_handler, request_body);
        }
        if(request_body.AsDict().at("type"s).AsString() == "Route"s) {
            result = AddRouteInfoResponse(router, request_body);
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

    json::Node AddRouteInfoResponse(const router::TransportRouter<double>& router, const json::Node& request_body) {
        std::optional<transport_catalogue::RouteInfo> route_info = router.GetResponse(request_body.AsDict().at("from"s).AsString(), request_body.AsDict().at("to"s).AsString());
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

    const json::Array& JsonReader::GetBaseRequests() const {
        return doc_.GetRoot().AsDict().at("base_requests"s).AsArray();
    }

    const json::Dict& JsonReader::GetRenderSettings() const {
        return doc_.GetRoot().AsDict().at("render_settings"s).AsDict();
    }

    const json::Array& JsonReader::GetStatRequests() const {
        return doc_.GetRoot().AsDict().at("stat_requests"s).AsArray();
    }
    
    const json::Dict& JsonReader::GetRouteSettings() const {
        return doc_.GetRoot().AsDict().at("routing_settings"s).AsDict();
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

    void SequentialRequestProcessing(transport_catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map_render, std::istream& input, std::ostream& output, request_handler::RequestHandler& request_handler) {
        JsonReader json_reader(input);
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

        router::TransportRouter<double> transport_router(
            request_handler, 
            json_reader.GetRouteSettings().at("bus_velocity"s).AsDouble(), 
            json_reader.GetRouteSettings().at("bus_wait_time"s).AsInt()
        );



        renderer::MapVisualizationSettings settings(
            json_reader.GetRenderSettings().at("width"s).AsDouble(), 
            json_reader.GetRenderSettings().at("height"s).AsDouble(), 
            json_reader.GetRenderSettings().at("padding"s).AsDouble()
        );
        auto route_coordinates = request_handler.GetCoordinates();
        renderer::SphereProjector projector(
            route_coordinates.begin(), 
            route_coordinates.end(), 
            settings.max_width, 
            settings.max_height, 
            settings.padding
        );
        map_render.SetPossibleColors(json_reader.GetRenderSettings().at("color_palette"s).AsArray());

        for(const auto& bus_name : json_reader.GetBusNames()) {
            const auto& bus = request_handler.GetBus(bus_name);
            if(bus != nullptr) {
                size_t pos = (bus->stops.size()) / 2;
                map_render.AddNewPointByRouteName(bus_name, projector(bus->stops.front()->coordinates), json_reader.GetRenderSettings());
                map_render.AddNewTextForRoute(bus_name, projector(bus->stops.front()->coordinates), json_reader.GetRenderSettings());
                map_render.AddNewCircleForStop(bus->stops.front()->name, projector(bus->stops.front()->coordinates), json_reader.GetRenderSettings());
                map_render.AddNewTextForStop(bus->stops.front()->name, projector(bus->stops.front()->coordinates), json_reader.GetRenderSettings());
                if(bus->is_circle == false && bus->stops.at(pos) != bus->stops.front()) {
                    map_render.AddNewTextForRoute(bus_name, projector(bus->stops.at(pos)->coordinates), json_reader.GetRenderSettings());
                }
                for(size_t i = 1; i < bus->stops.size() - 1; ++i) {
                    map_render.AddNewPointByRouteName(bus_name, projector(bus->stops.at(i)->coordinates), json_reader.GetRenderSettings());
                    map_render.AddNewCircleForStop(bus->stops.at(i)->name, projector(bus->stops.at(i)->coordinates), json_reader.GetRenderSettings());
                    map_render.AddNewTextForStop(bus->stops.at(i)->name, projector(bus->stops.at(i)->coordinates), json_reader.GetRenderSettings());
                }
                map_render.AddNewPointByRouteName(bus_name, projector(bus->stops.back()->coordinates), json_reader.GetRenderSettings());
                map_render.ChangeCurrentColor();
            }
        }
        std::ostringstream svg_output;
        request_handler.RenderMap(svg_output);
        json::Array result;
        json::Builder responses;
        responses.StartArray();
        for(const auto& request_body : json_reader.GetStatRequests()) {
            json::Node value = GenerateResponse(request_handler, transport_router, request_body, svg_output);
            responses.Value(value);
        }
        responses.EndArray();
        result = responses.Build().AsArray();
        json::Print(json::Document{result}, output);
    }
}