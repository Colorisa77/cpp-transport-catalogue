#include "transport_router.h"

using namespace std::literals;

namespace router {

    void TransportRouter::FillTransportRouter(const transport_catalogue::TransportCatalogue& transport_catalogue, const double velocity, const int wait_time) {

        SetVelocity(velocity);
        SetWaitTime(wait_time);

        graph_ = graph::DirectedWeightedGraph<double>(transport_catalogue.GetAllStopsFromCatalogue().size() * 2);

        FillGraphWithStops(transport_catalogue);
        FillGraphWithStopToStopDistance(transport_catalogue);

        router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    void TransportRouter::SetVelocity(const double velocity) {
        velocity_ = velocity;
    }

    void TransportRouter::SetWaitTime(const int wait_time) {
        wait_time_ = wait_time;
    }

    std::optional<transport_catalogue::RouteInfo> TransportRouter::GetRouteInfo(const std::string_view from, const std::string_view to) const {
        if(ids_by_stop_name_.count(from) == 0 || ids_by_stop_name_.count(to) == 0) {
            return std::nullopt;
        }
        std::optional<typename graph::Router<double>::RouteInfo> build_result = router_->BuildRoute((ids_by_stop_name_.at(from)).in, (ids_by_stop_name_.at(to)).in);
        if(build_result) {
            transport_catalogue::RouteInfo result;
            result.total_time = build_result.value().weight;
            for(const graph::EdgeId edge_id : build_result.value().edges) {
                auto value = action_by_edge_id_.at(edge_id);
                transport_catalogue::VertexToVertex path;
                if(std::holds_alternative<WaitBus>(value)) {
                    WaitBus w_bus = std::get<WaitBus>(action_by_edge_id_.at(edge_id));
                    path.type = "Wait"s;
                    path.bus_or_stop_name = w_bus.stop_name;
                    path.time = w_bus.time;
                    result.list_of_items.push_back(path);
                } else {
                    RideBus r_bus = std::get<RideBus>(action_by_edge_id_.at(edge_id));
                    path.type = "Bus"s;
                    path.bus_or_stop_name = r_bus.bus_name;
                    path.span = r_bus.span;
                    path.time = r_bus.time;
                    result.list_of_items.push_back(path);
                }
            }
            return result;
        }
        return std::nullopt;
    }

    void TransportRouter::FillGraphWithStops(const transport_catalogue::TransportCatalogue& transport_catalogue) {
        graph::VertexId in = 0;
        graph::VertexId out = 0;
        for(const std::string_view stop : transport_catalogue.GetAllStopsFromCatalogue()) {
            VertexIds ids{in, ++out};
            ids_by_stop_name_[stop] = ids;
            AddWaitingAction(stop, ids);
            in = out + 1;
            out = in;
        }
    }

    void TransportRouter::FillGraphWithStopToStopDistance(const transport_catalogue::TransportCatalogue& transport_catalogue) {
        for(std::string_view bus_name : transport_catalogue.GetAllBusesFromCatalogue()) {
            auto bus = transport_catalogue.GetBusByName((std::string)bus_name);
            auto it_from = bus->stops.begin();
            while(it_from != bus->stops.end()) {
                VertexIds ids_from;
                ids_from.in = ids_by_stop_name_.at((*it_from)->name).in;
                ids_from.out = ids_by_stop_name_.at((*it_from)->name).out;

                int span = 0;
                double distance = 0.0;

                auto it_to = it_from;
                ++it_to;
                auto prev_it = it_from;
                while(it_to != bus->stops.end()) {
                    VertexIds ids_to;
                    ids_to.in = ids_by_stop_name_.at((*it_to)->name).in;
                    ids_to.out = ids_by_stop_name_.at((*it_to)->name).out;
                    distance += (transport_catalogue.GetStopToStopDistanceByName((*prev_it)->name, (*it_to)->name)) / (velocity_ * METERS_PER_KM / MINUTES_IN_HOUR);
                    AddRidingAction(bus->name, ids_from.out, ids_to.in, ++span, distance); 
                    ++prev_it;
                    ++it_to;
                }
                ++it_from;
            }
        }
    }

    void TransportRouter::AddWaitingAction(const std::string_view stop_name, const VertexIds ids) {
        graph::EdgeId edge_id = graph_.AddEdge({ids.in, ids.out, (double)wait_time_});
        action_by_edge_id_[edge_id] = WaitBus{stop_name, (double)wait_time_};
    }

    void TransportRouter::AddRidingAction(const std::string_view bus_name, const graph::VertexId from, const graph::VertexId to, const int span, const double distance) {
        graph::EdgeId edge_id = graph_.AddEdge({from, to, distance});
        action_by_edge_id_[edge_id] = RideBus{bus_name, span, distance};
    }

} // namespace router