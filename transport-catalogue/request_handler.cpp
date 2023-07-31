#include "request_handler.h"

namespace request_handler {
    RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& transport_catalogue, const router::TransportRouter& router, const renderer::MapRenderer& map_render) 
        :db_(std::move(transport_catalogue))
        ,router_(std::move(router))
        ,renderer_(std::move(map_render)) {
    }
    
    std::optional<transport_catalogue::BusInfo> RequestHandler::GetBusInfo(const std::string_view bus_name) const {
        return db_.GetBusInfo(bus_name);
    }

    std::optional<transport_catalogue::StopInfo> RequestHandler::GetStopInfo(const std::string_view stop_name) const {
        return db_.GetStopInfo(stop_name);
    }

    std::optional<transport_catalogue::RouteInfo> RequestHandler::GetRouteInfo(const std::string_view from, const std::string_view to) const {
        return router_.GetRouteInfo(from, to);
    }

    std::vector<std::string_view> RequestHandler::GetAllBusesFromCatalogue() const {
        return db_.GetAllBusesFromCatalogue();
    }

    const transport_catalogue::Bus* RequestHandler::GetBus(const std::string_view bus_name) const {
        return db_.GetBusByName(std::string(bus_name));
    } 

    transport_catalogue::Stop RequestHandler::GetStop(const std::string_view stop_name) const { 
        return db_.GetStopByName(stop_name); 
    }


    std::vector<geo::Coordinates> RequestHandler::GetCoordinatesFromStopsWithCoordinates() const {
        return db_.GetCoordinatesFromStopsWithCoordinates();
    }

    void RequestHandler::RenderMap(std::ostream& output) const {
        renderer_.Render(output);
    }
}