#pragma once
#include <iostream>
#include "request_handler.h"
#include "json_builder.h"
#include "json.h"
#include "transport_router.h"

#include <set>
using namespace std::literals;

namespace json_reader {

    class JsonReader {
    public:
        JsonReader(std::istream& input);

        const json::Array& GetBaseRequests() const;
        const json::Dict& GetRenderSettings() const;
        const json::Array& GetStatRequests() const;
        const json::Dict& GetRouteSettings() const;

        void AddRouteCoordinates(geo::Coordinates coordinates);
        const std::vector<geo::Coordinates>& GetRoutesCoordinate() const;

        void AddStopToStopRequest(const json::Node& node);
        void AddBusRequest(const json::Node& node);
        const Requests& GetRequests() const;

        void AddBusName(const std::string& bus_name);
        const std::set<std::string>& GetBusNames() const;

    private:

        Requests requests_;
        json::Array responses_;
        json::Document doc_;
        std::vector<geo::Coordinates> route_coordinates_;
        std::set<std::string> buses_names_;
    };

    std::vector<geo::Coordinates> GetStopsCoordinates();

    void FillStopsByRequestBody(transport_catalogue::TransportCatalogue& transport_catalogue, const json::Node& request_body, Requests& requests);
    void FillStopToStopDistances(transport_catalogue::TransportCatalogue& transport_catalogue, const json::Node& stop_to_stop_distance);
    void FillRoutesByRequestBody(transport_catalogue::TransportCatalogue& transport_catalogue, const json::Node& add_buses_request);

    json::Node GenerateResponse(const request_handler::RequestHandler& request_handler, const json::Node& request_body, std::ostream& output);

    json::Node AddStopInfoResponse(const request_handler::RequestHandler& request_handler, const json::Node& request_body);
    json::Node AddBusInfoResponse(const request_handler::RequestHandler& request_handler, const json::Node& request_body);
    json::Node AddRouteInfoResponse(const request_handler::RequestHandler& request_handler, const json::Node& request_body);


    void FillingTransportCatalogue(transport_catalogue::TransportCatalogue& transport_catalogue) ;
    void SequentialRequestProcessing(
        transport_catalogue::TransportCatalogue& transport_catalogue, 
        router::TransportRouter& router, 
        renderer::MapRenderer& map_render, 
        std::istream& input, 
        std::ostream& output, 
        request_handler::RequestHandler& request_handler
    );
}