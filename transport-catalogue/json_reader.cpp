#include "json_reader.h"
#include "map_renderer.h"
#include <sstream>
using namespace std::literals;

namespace json_reader {
    void FillStopsByRequestBody(transport_catalogue::TransportCatalogue& transport_catalogue, const json::Node& request_body) {
        geo::Coordinates coordinates {
            request_body.AsMap().at("latitude"s).AsDouble(), 
            request_body.AsMap().at("longitude"s).AsDouble()
        };
        transport_catalogue.AddStop(request_body.AsMap().at("name"s).AsString(), coordinates);
    }

    void FillStopToStopDistances(transport_catalogue::TransportCatalogue& transport_catalogue, const json::Node& stop_to_stop_distance) {
        for(const auto& [second_stop_name, distance] : stop_to_stop_distance.AsMap().at("road_distances"s).AsMap()) {
            transport_catalogue.SetStopToStopDistances(
                stop_to_stop_distance.AsMap().at("name"s).AsString(), 
                second_stop_name, 
                distance.AsDouble()
            );
        }
    }

    void FillRoutesByRequestBody(transport_catalogue::TransportCatalogue& transport_catalogue, const json::Node& add_buses_request) {
        std::vector<std::string> stops;
        for(const auto& stop : add_buses_request.AsMap().at("stops"s).AsArray()) {
            stops.push_back(stop.AsString());
        }
        transport_catalogue.AddBus(
            add_buses_request.AsMap().at("name"s).AsString(),
            stops, 
            add_buses_request.AsMap().at("is_roundtrip"s).AsBool()
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

    void AddSvgOutput(const std::ostringstream& svg_output, const json::Node& request_body, json::Array& responses) {
        json::Dict response {
            {"request_id"s, request_body.AsMap().at("id"s).AsInt()},
            {"map"s, EscapingAString(svg_output)}
        };
        responses.push_back(response);
    }


    void GenerateResponse(request_handler::RequestHandler& request_handler, const json::Node& request_body, json::Array& responses, const std::ostringstream& svg_output) {
        if(request_body.AsMap().at("type"s).AsString() == "Stop"s) {
            AddStopInfoResponse(request_handler, request_body, responses);
        }
        if(request_body.AsMap().at("type"s).AsString() == "Bus"s) {
            AddBusInfoResponse(request_handler, request_body, responses);
        }
        if(request_body.AsMap().at("type"s).AsString() == "Map"s) {
            AddSvgOutput(svg_output, request_body, responses);
        }
    }

    void AddStopInfoResponse(request_handler::RequestHandler& request_handler, const json::Node& request_body, json::Array& responses) {
        std::optional<transport_catalogue::StopInfo> stop_info = request_handler.GetStopInfo(request_body.AsMap().at("name"s).AsString());
        if(stop_info && stop_info.value().is_empty == false) {
            json::Array stop_buses;
            for(const auto& bus : stop_info.value().stop_buses) {
                std::string string_result = {bus.begin(), bus.end()};
                stop_buses.push_back(string_result);
            }
            json::Dict response {
                {"buses"s, stop_buses},
                {"request_id"s, request_body.AsMap().at("id"s).AsInt()}
            };
            responses.push_back(response);
            return;
        }
        json::Dict response {
            {"request_id"s, request_body.AsMap().at("id"s).AsInt()},
            {"error_message"s, "not found"s}
        };
        responses.push_back(response);
        return;
    }

    void AddBusInfoResponse(request_handler::RequestHandler& request_handler, const json::Node& request_body, json::Array& responses) {
        std::optional<transport_catalogue::BusInfo> bus_info = request_handler.GetBusInfo(request_body.AsMap().at("name"s).AsString());
        if(bus_info && bus_info.value().is_empty == false) {
            json::Dict response {
                {"curvature"s, bus_info.value().curvature},
                {"request_id"s, request_body.AsMap().at("id"s).AsInt()},
                {"route_length"s, bus_info.value().route_length},
                {"stop_count"s, static_cast<int>(bus_info.value().count_all_stops)},
                {"unique_stop_count"s, static_cast<int>(bus_info.value().count_unique_stops)}
            };
            responses.push_back(response);
            return;
        }
        json::Dict response {
            {"request_id"s, request_body.AsMap().at("id"s).AsInt()},
            {"error_message"s, "not found"s}
        };
        responses.push_back(response);
        return;
    }


    JsonReader::JsonReader(std::istream& input) 
        : doc_(json::Load(input)) {
    }

    const json::Array& JsonReader::GetBaseRequests() const {
        return doc_.GetRoot().AsMap().at("base_requests"s).AsArray();
    }

    const json::Dict& JsonReader::GetRenderSettings() const {
        return doc_.GetRoot().AsMap().at("render_settings"s).AsMap();
    }

    const json::Array& JsonReader::GetStatRequests() const {
        return doc_.GetRoot().AsMap().at("stat_requests"s).AsArray();
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
            if(request_body.AsMap().at("type"s).AsString() == "Stop"s) {
                FillStopsByRequestBody(transport_catalogue, request_body);
                json::Node stop_to_stop_distance_request = json::Dict{
                    {"name"s, request_body.AsMap().at("name"s).AsString()},
                    {"road_distances"s, request_body.AsMap().at("road_distances"s).AsMap()}
                };
                json_reader.AddStopToStopRequest(stop_to_stop_distance_request);
                json_reader.AddRouteCoordinates({
                    request_body.AsMap().at("latitude"s).AsDouble(), 
                    request_body.AsMap().at("longitude"s).AsDouble()
                });
                continue;
            }
            if(request_body.AsMap().at("type"s).AsString() == "Bus"s) {
                json_reader.AddBusRequest(request_body);
            }
        }

        for(const auto& add_stop_to_stop_distance_request : json_reader.GetRequests().stop_requests) {
            FillStopToStopDistances(transport_catalogue, add_stop_to_stop_distance_request);
        }


        for(const auto& add_buses_request : json_reader.GetRequests().bus_requests) {
            FillRoutesByRequestBody(transport_catalogue, add_buses_request);
            json_reader.AddBusName(add_buses_request.AsMap().at("name"s).AsString());

        } 

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
        json::Array responses;
        for(const auto& request_body : json_reader.GetStatRequests()) {
            GenerateResponse(request_handler, request_body, responses, svg_output);
        }
        json::Print(json::Document{responses}, output);
    }
}