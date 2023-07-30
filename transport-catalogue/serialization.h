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

    void SerializeTransportCatalogue(transport_catalogue_proto::Catalogue& catalogue, const transport_catalogue::TransportCatalogue& transport_catalogue, json_reader::JsonReader& json_reader);
    void SerializeColor(svg::Color& svg_color, transport_catalogue_proto::Color& color_proto);
    void SerializeRenderSettings(transport_catalogue_proto::Catalogue& catalogue, const renderer::RenderSettings& render_settings);
    void DeserializeTransportCatalogue(transport_catalogue::TransportCatalogue& transport_catalogue, json_reader::JsonReader& json_reader);
    void DeserializeRenderSettings(const renderer::MapRenderer& map_renderer, json_reader::JsonReader& json_reader);

} // namespace serialization