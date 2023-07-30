#include "serialization.h"

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

        std::vector<transport_catalogue_proto::Stop> stops_proto_vector(transport_catalogue.GetAllStopsFromCatalogue().size());
        //std::vector<transport_catalogue_proto::Bus> buses_proto_vector(transport_catalogue.GetAllBusesFromCatalogue().size());
        //std::vector<transport_catalogue_proto::StopToStopDistance> stop_to_stop_distances_proto(transport_catalogue.GetAllDistances().size());

        std::unordered_map<std::string, transport_catalogue_proto::Stop*> stop_indexes(transport_catalogue.GetAllStopsFromCatalogue().size());

        int curr_id = 0;

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

        curr_id = 0;

        for(const std::string_view bus_name : transport_catalogue.GetAllBusesFromCatalogue()) {
            std::string b_name = std::string(bus_name);
            const auto& bus = transport_catalogue.GetBusByName(std::string(b_name));

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
        (void)transport_catalogue;
    }
} // namespace serialization