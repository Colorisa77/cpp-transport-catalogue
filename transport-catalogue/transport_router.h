#pragma once

#include "domain.h"
#include "request_handler.h"
#include "router.h"
#include "graph.h"

#include <variant>
#include <memory>

namespace router {

    struct VertexIds {
        graph::VertexId in;
        graph::VertexId out;
    };

    template<typename Weight>
    struct WaitBus {
        std::string_view stop_name;
        Weight time;
    };

    template<typename Weight>
    struct RideBus {
        std::string_view bus_name;
        int span;
        Weight time;
    };

    template<typename Weight>
    class TransportRouter {
        using Wait = WaitBus<Weight>;
        using Ride = RideBus<Weight>;
    public:
        TransportRouter(const request_handler::RequestHandler& request_handler, const double velocity, const int wait_time) 
        : VELOCITY(velocity)
        , WAIT_TIME(wait_time) {

            graph_ = std::move(graph::DirectedWeightedGraph<Weight>(request_handler.GetAllStops().size() * 2));

            SetVertices(request_handler);
            SetEdges(request_handler);

            router_ = std::make_unique<graph::Router<Weight>>(graph_);
        }

        std::optional<transport_catalogue::RouteInfo> GetResponse(const std::string_view from, const std::string_view to) const {
            if(ids_by_stop_name_.count(from) == 0 || ids_by_stop_name_.count(to) == 0) {
                return std::nullopt;
            }
            std::optional<typename graph::Router<Weight>::RouteInfo> build_result = router_->BuildRoute((ids_by_stop_name_.at(from)).in, (ids_by_stop_name_.at(to)).in);
            if(build_result) {
                transport_catalogue::RouteInfo result;
                result.total_time = build_result.value().weight;
                for(const graph::EdgeId edge_id : build_result.value().edges) {
                    auto value = action_by_edge_id_.at(edge_id);
                    transport_catalogue::VertexToVertex path;
                    if(std::holds_alternative<Wait>(value)) {
                        Wait w_bus = std::get<Wait>(action_by_edge_id_.at(edge_id));
                        path.type = "Wait"s;
                        path.bus_or_stop_name = w_bus.stop_name;
                        path.time = w_bus.time;
                        result.list_of_items.push_back(path);
                    } else {
                        Ride r_bus = std::get<Ride>(action_by_edge_id_.at(edge_id));
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

    private:
        const double VELOCITY;
        const int WAIT_TIME;
        const int METERS_PER_KM = 1000;
        const int MINUTES_IN_HOUR = 60;
        graph::DirectedWeightedGraph<Weight> graph_;
        std::unique_ptr<graph::Router<Weight>> router_ = nullptr;
        std::unordered_map<std::string_view, VertexIds> ids_by_stop_name_;
        std::unordered_map<graph::EdgeId, std::variant<Wait, Ride>> action_by_edge_id_;


        void SetVertices(const request_handler::RequestHandler& request_handler) {
            graph::VertexId in = 0;
            graph::VertexId out = 0;
            for(const auto& stop : request_handler.GetAllStops()) {
                VertexIds ids{in, ++out};
                ids_by_stop_name_[stop.name] = ids;
                AddWaitingEdge(stop.name, ids);
                in = out + 1;
                out = in;
            }
        }

        void SetEdges(const request_handler::RequestHandler& request_handler) {
            for(const auto& bus : request_handler.GetAllBuses()) {
                auto it_from = bus.stops.begin();
                while(it_from != bus.stops.end()) {
                    VertexIds ids_from;
                    ids_from.in = ids_by_stop_name_.at((*it_from)->name).in;
                    ids_from.out = ids_by_stop_name_.at((*it_from)->name).out;

                    int span = 0;
                    double distance = 0.0;

                    auto it_to = it_from;
                    ++it_to;
                    auto prev_it = it_from;
                    while(it_to != bus.stops.end()) {
                        VertexIds ids_to;
                        ids_to.in = ids_by_stop_name_.at((*it_to)->name).in;
                        ids_to.out = ids_by_stop_name_.at((*it_to)->name).out;
                        distance += (request_handler.GetStopToStopDistance((*prev_it)->name, (*it_to)->name)) / (VELOCITY * METERS_PER_KM / MINUTES_IN_HOUR);
                        AddRidingEdge(bus.name, ids_from.out, ids_to.in, ++span, distance); 
                        ++prev_it;
                        ++it_to;
                    }
                    ++it_from;
                }
            }
        }

        void AddWaitingEdge(const std::string_view stop_name, const VertexIds ids) {
            graph::EdgeId edge_id = graph_.AddEdge({ids.in, ids.out, (double)WAIT_TIME});
            action_by_edge_id_[edge_id] = Wait{stop_name, (double)WAIT_TIME};
        }

        void AddRidingEdge(const std::string_view bus_name, const graph::VertexId from, const graph::VertexId to, const int span, const double distance) {
            graph::EdgeId edge_id = graph_.AddEdge({from, to, distance});
            action_by_edge_id_[edge_id] = Ride{bus_name, span, distance};
        }
    };

} //router