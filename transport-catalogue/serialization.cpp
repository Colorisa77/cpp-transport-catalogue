#include "serialization.h"
#include "domain.h"

#include <transport_catalogue.pb.h>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace serialization {
    void SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& transport_catalogue, json_reader::JsonReader& json_reader) {
        std::string file_name = json_reader.GetSerializationSettingsRequests().at("file"s).AsString();
        std::ofstream out_file(file_name, std::ios::binary);
        transport_catalogue_proto::TransportCatalogue transport_catalogue_proto;

        std::vector<transport_catalogue_proto::Stop> stops_proto_vector;
        stops_proto_vector.reserve(transport_catalogue.GetAllStopsFromCatalogue().size());

        std::unordered_map<std::string, transport_catalogue_proto::Stop*> stop_indexes;
        stop_indexes.reserve(transport_catalogue.GetAllStopsFromCatalogue().size());

        int curr_id = 1;

        for(const std::string_view stop_name : transport_catalogue.GetAllStopsFromCatalogue()) {
            const auto& stop = transport_catalogue.GetStopByName(stop_name);

            transport_catalogue_proto::Stop stop_proto;
            stop_proto.set_id(curr_id);
            stop_proto.set_name(std::string(stop_name));

            transport_catalogue_proto::Coordinates coordinates_proto;
            coordinates_proto.set_lat(stop.coordinates.lat);
            coordinates_proto.set_lng(stop.coordinates.lng);

            stop_proto.mutable_coordinates()->Swap(&coordinates_proto);

            stops_proto_vector.emplace_back((stop_proto));
            stop_indexes[std::string(stop_name)] = &stops_proto_vector.back();
            transport_catalogue_proto.add_stops()->Swap(&stop_proto);
            ++curr_id;
        }

        curr_id = 1;

        for(const std::string_view bus_name : transport_catalogue.GetAllBusesFromCatalogue()) {
            std::string b_name = std::string(bus_name);
            const auto bus = transport_catalogue.GetBusByName(std::string(b_name));

            transport_catalogue_proto::Bus bus_proto;
            bus_proto.set_id(curr_id);
            bus_proto.set_name(b_name);

            for(const auto stop : bus->stops) {
                bus_proto.add_stop_ids(stop_indexes.at(std::string(stop->name))->id());
            }

            bus_proto.set_is_circle(bus->is_circle);

            transport_catalogue_proto.add_buses()->Swap(&bus_proto);
            ++curr_id;
        }

        const auto& stop_to_stop_distances = transport_catalogue.GetAllDistances();
        for(const auto& [from_stop_to_stop, distance] : stop_to_stop_distances) {
            std::string from_stop = std::string(from_stop_to_stop.first->name);
            std::string to_stop = std::string(from_stop_to_stop.second->name);
            transport_catalogue_proto::StopToStopDistance stop_to_stop_distance_proto;

            stop_to_stop_distance_proto.set_from_stop_id(stop_indexes.at(from_stop)->id());
            stop_to_stop_distance_proto.set_to_stop_id(stop_indexes.at(to_stop)->id());
            stop_to_stop_distance_proto.set_distance(distance);

            transport_catalogue_proto.add_stop_to_stop_distances()->Swap(&stop_to_stop_distance_proto);
        }

        transport_catalogue_proto.SerializeToOstream(&out_file);
    }
    void DeserializeTransportCatalogue(transport_catalogue::TransportCatalogue& transport_catalogue, json_reader::JsonReader& json_reader) {
        std::string file_name = json_reader.GetSerializationSettingsRequests().at("file"s).AsString();
        std::ifstream in_file(file_name, std::ios::binary);
        transport_catalogue_proto::TransportCatalogue transport_catalogue_proto;

        std::unordered_map<uint32_t, std::string> stop_indexes;
        stop_indexes.reserve(transport_catalogue_proto.stops_size());

        if(transport_catalogue_proto.ParseFromIstream(&in_file)) {
            for(auto& stop_proto : transport_catalogue_proto.stops()) {
                stop_indexes[stop_proto.id()] = stop_proto.name();
                transport_catalogue.AddStop(stop_proto.name(), {stop_proto.coordinates().lat(), stop_proto.coordinates().lng()});
            }

            for(auto& bus_proto : transport_catalogue_proto.buses()) {
                std::vector<std::string> stops;
                stops.reserve(bus_proto.stop_ids().size());
                for(uint32_t stop_id : bus_proto.stop_ids()) {
                    std::string stop_name = stop_indexes.at(stop_id);
                    stops.push_back(stop_name);
                }
                transport_catalogue.AddBus(bus_proto.name(), stops, true);
                if(!bus_proto.is_circle()) {
                    transport_catalogue.ChangeLastRouteRoundTrip();
                }
            }

            for(const auto& stop_to_stop_distance_proto : transport_catalogue_proto.stop_to_stop_distances()) {
                std::string from_stop_name = stop_indexes.at(stop_to_stop_distance_proto.from_stop_id());
                std::string to_stop_name = stop_indexes.at(stop_to_stop_distance_proto.to_stop_id());

                transport_catalogue.SetStopToStopDistances(from_stop_name, to_stop_name, stop_to_stop_distance_proto.distance());
            }
        }
    }
} // namespace serialization