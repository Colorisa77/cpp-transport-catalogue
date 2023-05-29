#include "request_handler.h"

namespace request_handler {
    RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& transport_catalogue, const renderer::MapRenderer& map_render) 
        :db_(std::move(transport_catalogue))
        ,renderer_(std::move(map_render)) {
    }
    
    std::optional<transport_catalogue::BusInfo> RequestHandler::GetBusInfo(const std::string_view bus_name) const {
        return db_.GetBusInfo(bus_name);
    }

    std::optional<transport_catalogue::StopInfo> RequestHandler::GetStopInfo(const std::string_view stop_name) const {
        return db_.GetStopInfo(stop_name);
    }

    const transport_catalogue::Bus* RequestHandler::GetBus(const std::string bus_name) const {
        return db_.GetBusByName(bus_name);
    }

    transport_catalogue::Stop RequestHandler::GetStop(const std::string_view stop_name) const {
        return db_.GetStopByName(stop_name);
    }

    std::vector<geo::Coordinates> RequestHandler::GetCoordinates() const {
        return db_.GetStopsWithCoordinates();
    }

    double RequestHandler::GetStopToStopDistance(const std::string_view from, const std::string_view to) const {
        return db_.GetStopToStopDistanceByName(from, to);
    }

    const std::deque<transport_catalogue::Bus>& RequestHandler::GetAllBuses() const {
        return db_.GetAllBuses();
    }

    const std::deque<transport_catalogue::Stop>& RequestHandler::GetAllStops() const {
        return db_.GetAllStops();
    }

    void RequestHandler::RenderMap(std::ostream& output) const {
        renderer_.Render(output);
    }
}