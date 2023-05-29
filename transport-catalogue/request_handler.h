#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "json.h"
#include <optional>

struct Requests {
    std::deque<json::Node> stop_requests;
    std::deque<json::Node> bus_requests;
};

namespace request_handler {
        class RequestHandler {
    public:
        RequestHandler(const transport_catalogue::TransportCatalogue& transport_catalogue, const renderer::MapRenderer& map_render);

        std::optional<transport_catalogue::BusInfo> GetBusInfo(const std::string_view bus_name) const;
        std::optional<transport_catalogue::StopInfo> GetStopInfo(const std::string_view stop_name) const;

        std::vector<geo::Coordinates> GetCoordinates() const;

        const transport_catalogue::Bus* GetBus(const std::string bus_name) const;
        transport_catalogue::Stop GetStop(const std::string_view stop_name) const;

        const std::deque<transport_catalogue::Bus>& GetAllBuses() const;
        const std::deque<transport_catalogue::Stop>& GetAllStops() const;

        double GetStopToStopDistance(const std::string_view from, const std::string_view to) const;

        void RenderMap(std::ostream& output) const;

    private:
        const transport_catalogue::TransportCatalogue& db_;
        const renderer::MapRenderer& renderer_;
    };
}