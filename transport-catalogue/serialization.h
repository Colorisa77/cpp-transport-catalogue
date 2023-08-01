#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include <transport_catalogue.pb.h>
#include <filesystem>

namespace serialization {
    struct Catalogue {
        transport_catalogue::TransportCatalogue transport_catalogue;
        renderer::MapRenderer map_renderer;
    };

    void SerializeTransportCatalogue(transport_catalogue_proto::Catalogue& catalogue, const transport_catalogue::TransportCatalogue& transport_catalogue);
    void SerializeColor(svg::Color& svg_color, transport_catalogue_proto::Color& color_proto);
    void SerializeRenderSettings(transport_catalogue_proto::Catalogue& catalogue, const renderer::RenderSettings& render_settings);
    void SerializeRouteSettings(transport_catalogue_proto::Catalogue& catalogue, const graph::RouteSettings& routing_settings);

    void SerializeCatalogue(const transport_catalogue_proto::Catalogue& catalogue, json_reader::JsonReader& json_reader);
    void DeserializeCatalogue(transport_catalogue_proto::Catalogue& catalogue_proto, json_reader::JsonReader& json_reader);

    svg::Color DeserializeUnderlayerColor(const transport_catalogue_proto::Color& catalogue_proto);
    svg::Color DeserializeColorPalette(const transport_catalogue_proto::Color& catalogue_proto);
    void DeserializeTransportCatalogue(transport_catalogue::TransportCatalogue& transport_catalogue, transport_catalogue_proto::Catalogue& catalogue_proto);
    void DeserializeRenderSettings(renderer::RenderSettings& render_settings, transport_catalogue_proto::Catalogue& catalogue_proto);
    void DeserializeRouteSettings(graph::RouteSettings& routing_settings, transport_catalogue_proto::Catalogue& catalogue_proto);

} // namespace serialization