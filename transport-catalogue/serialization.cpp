#include "serialization.h"

#include <transport_catalogue.pb.h>
#include <fstream>
#include <string>
#include <string_view>

namespace serialization {
    void SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& transport_catalogue, json_reader::JsonReader& json_reader) {
        std::string file_name = json_reader.GetSerializationSettingsRequests().at("file"s).AsString();
        std::ofstream out_file(file_name, std::ios::binary);
        transport_catalogue_proto::TransportCatalogue transport_catalogue_proto;
        //transport_catalogue_proto.set_file_name(std::string(file_name));

        for(const std::string_view stop_name : transport_catalogue.GetAllStopsFromCatalogue()) {
            const auto& stop = transport_catalogue.GetStopByName(stop_name);

            transport_catalogue_proto::Stop stop_proto;
            stop_proto.set_name(std::string(stop_name));

            transport_catalogue_proto::Coordinates coordinates_proto;
            coordinates_proto.set_lat(stop.coordinates.lat);
            coordinates_proto.set_lng(stop.coordinates.lng);

            stop_proto.mutable_coordinates()->Swap(&coordinates_proto);
        }

        for(const std::string_view bus_name : transport_catalogue.GetAllBusesFromCatalogue()) {
            const auto& bus = transport_catalogue.GetBusByName(std::string(bus_name));
            (void)bus;

        }
    }
    void DeserializeTransportCatalogue(transport_catalogue::TransportCatalogue& transport_catalogue) {
        (void)transport_catalogue;
    }
} // namespace serialization