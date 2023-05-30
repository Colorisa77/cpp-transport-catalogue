#pragma once

#include "domain.h"
#include "router.h"
#include "graph.h"

#include "transport_catalogue.h"

#include <variant>
#include <memory>

namespace router {

    struct VertexIds {
        graph::VertexId in;
        graph::VertexId out;
    };

    struct WaitBus {
        std::string_view stop_name;
        double time;
    };

    struct RideBus {
        std::string_view bus_name;
        int span;
        double time;
    };

    class TransportRouter {
    public:
        void FillTransportRouter(const transport_catalogue::TransportCatalogue& transport_catalogue, const double velocity, const int wait_time);

        void SetVelocity(const double velocity);
        void SetWaitTime(const int wait_time);

        std::optional<transport_catalogue::RouteInfo> GetRouteInfo(const std::string_view from, const std::string_view to) const;

    private:
        double velocity_ = 0.0;
        int wait_time_ = 0;
        const int METERS_PER_KM = 1000;
        const int MINUTES_IN_HOUR = 60;
        graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<graph::Router<double>> router_ = nullptr;
        std::unordered_map<std::string_view, VertexIds> ids_by_stop_name_;
        std::unordered_map<graph::EdgeId, std::variant<WaitBus, RideBus>> action_by_edge_id_;


        void FillGraphWithStops(const transport_catalogue::TransportCatalogue& transport_catalogue);
        void FillGraphWithStopToStopDistance(const transport_catalogue::TransportCatalogue& transport_catalogue);

        void AddWaitingAction(const std::string_view stop_name, const VertexIds ids);
        void AddRidingAction(const std::string_view bus_name, const graph::VertexId from, const graph::VertexId to, const int span, const double distance);
    };

} //router