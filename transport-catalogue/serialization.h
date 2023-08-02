#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include <transport_catalogue.pb.h>
#include <filesystem>

namespace serialization {
    using StopIndexes = std::unordered_map<std::string, transport_catalogue_proto::Stop>;
    using StopIds = std::unordered_map<uint32_t, std::string>;

    struct Catalogue {
        transport_catalogue::TransportCatalogue transport_catalogue;
        renderer::MapRenderer map_renderer;
    };

    void SerializeStops(transport_catalogue_proto::TransportCatalogue& transport_catalogue_proto, const transport_catalogue::TransportCatalogue& transport_catalogue, StopIndexes& stop_indexes);
    void SerializeBuses(transport_catalogue_proto::TransportCatalogue& transport_catalogue_proto, const transport_catalogue::TransportCatalogue& transport_catalogue, const StopIndexes& stop_indexes);
    void SerializeStopToStopDistance(transport_catalogue_proto::TransportCatalogue& transport_catalogue_proto, const transport_catalogue::TransportCatalogue& transport_catalogue, const StopIndexes& stop_indexes);
    void SerializeTransportCatalogue(transport_catalogue_proto::Catalogue& catalogue, const transport_catalogue::TransportCatalogue& transport_catalogue);

    void SerializeColor(svg::Color& svg_color, transport_catalogue_proto::Color& color_proto);
    void SerializeRenderSettings(transport_catalogue_proto::Catalogue& catalogue, const renderer::RenderSettings& render_settings);

    void SerializeRouteSettings(transport_catalogue_proto::Catalogue& catalogue, const graph::RouteSettings& routing_settings);

    void SerializeCatalogue(const transport_catalogue_proto::Catalogue& catalogue, json_reader::JsonReader& json_reader);
    void DeserializeCatalogue(transport_catalogue_proto::Catalogue& catalogue_proto, json_reader::JsonReader& json_reader);

    svg::Color DeserializeUnderlayerColor(const transport_catalogue_proto::Color& catalogue_proto);
    svg::Color DeserializeColorPalette(const transport_catalogue_proto::Color& catalogue_proto);
    void DeserializeRenderSettings(renderer::RenderSettings& render_settings, transport_catalogue_proto::Catalogue& catalogue_proto);

    void DeserializeTransportCatalogue(transport_catalogue::TransportCatalogue& transport_catalogue, const transport_catalogue_proto::Catalogue& catalogue_proto);
    StopIds DeserializeStops(transport_catalogue::TransportCatalogue& transport_catalogue, const transport_catalogue_proto::Catalogue& catalogue_proto);
    void DeserializeBuses(transport_catalogue::TransportCatalogue& transport_catalogue, const transport_catalogue_proto::Catalogue& catalogue_proto, const StopIds& stop_ids);
    void DeserializeStopToStopDistances(transport_catalogue::TransportCatalogue& transport_catalogue, const transport_catalogue_proto::Catalogue& catalogue_proto, const StopIds& stop_ids);

    void DeserializeRouteSettings(graph::RouteSettings& routing_settings, transport_catalogue_proto::Catalogue& catalogue_proto);

} // namespace serialization