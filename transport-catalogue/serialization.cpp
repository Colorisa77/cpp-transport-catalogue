#include "serialization.h"
#include "domain.h"

#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace serialization {
    void SerializeTransportCatalogue(transport_catalogue_proto::Catalogue& catalogue, const transport_catalogue::TransportCatalogue& transport_catalogue) {
        //std::string file_name = json_reader.GetSerializationSettingsRequests().at("file"s).AsString();
        //std::ofstream out_file(file_name, std::ios::binary);
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
        catalogue.mutable_transport_catalogue()->Swap(&transport_catalogue_proto);
    }
    void SerializeColor(svg::Color& svg_color, transport_catalogue_proto::Color& color_proto) {

        if (std::holds_alternative<std::monostate>(svg_color)) {
            color_proto.set_none(true);

        } else if (std::holds_alternative<svg::Rgb>(svg_color)) {
            svg::Rgb rgb = std::get<svg::Rgb>(svg_color);

            color_proto.mutable_rgb()->set_red(rgb.red);
            color_proto.mutable_rgb()->set_green(rgb.green);
            color_proto.mutable_rgb()->set_blue(rgb.blue);

        } else if (std::holds_alternative<svg::Rgba>(svg_color)) {
            svg::Rgba rgba = std::get<svg::Rgba>(svg_color);

            color_proto.mutable_rgba()->set_red(rgba.red);
            color_proto.mutable_rgba()->set_green(rgba.green);
            color_proto.mutable_rgba()->set_blue(rgba.blue);
            color_proto.mutable_rgba()->set_opacity(rgba.opacity);

        } else if (std::holds_alternative<std::string>(svg_color)) {
            color_proto.set_color_string(std::get<std::string>(svg_color));
        }
    }
    void SerializeRenderSettings(transport_catalogue_proto::Catalogue& catalogue, const renderer::RenderSettings& render_settings) {
        transport_catalogue_proto::RenderSettings render_settings_proto;

        render_settings_proto.set_width(render_settings.width);
        render_settings_proto.set_height(render_settings.height);
        render_settings_proto.set_padding(render_settings.padding);
        render_settings_proto.set_line_width(render_settings.line_width);
        render_settings_proto.set_stop_radius(render_settings.stop_radius);
        render_settings_proto.set_bus_label_font_size(render_settings.bus_label_font_size);

        transport_catalogue_proto::Point point_proto;
        point_proto.set_x(render_settings.bus_label_offset.x);
        point_proto.set_y(render_settings.bus_label_offset.y);
        render_settings_proto.mutable_bus_label_offset()->Swap(&point_proto);

        render_settings_proto.set_stop_label_font_size(render_settings.stop_label_font_size);

        point_proto.set_x(render_settings.stop_label_offset.x);
        point_proto.set_y(render_settings.stop_label_offset.y);
        render_settings_proto.mutable_stop_label_offset()->Swap(&point_proto);

        transport_catalogue_proto::Color color_proto;
        svg::Color clr(render_settings.underlayer_color);
        serialization::SerializeColor(clr, color_proto);
        render_settings_proto.mutable_underlayer_color()->Swap(&color_proto);

        render_settings_proto.set_underlayer_width(render_settings.underlayer_width);

        int i = 0;
        for(svg::Color color : render_settings.color_palette) {
            serialization::SerializeColor(color, color_proto);
            transport_catalogue_proto::ColorPalette color_palette_proto;
            color_palette_proto.set_id(i);
            color_palette_proto.mutable_color()->Swap(&color_proto);
            render_settings_proto.add_color_palette()->Swap(&color_palette_proto);
            ++i;
        }
        catalogue.mutable_render_settings()->Swap(&render_settings_proto);
    }

    void SerializeCatalogue(const transport_catalogue_proto::Catalogue& catalogue, json_reader::JsonReader& json_reader) {
        std::string file = json_reader.GetSerializationSettingsRequests().at("file"s).AsString();
        std::ofstream out_file(file, std::ios::binary);
        catalogue.SerializeToOstream(&out_file);
    }

    void DeserializeCatalogue(transport_catalogue_proto::Catalogue& catalogue_proto, json_reader::JsonReader& json_reader) {
        std::string file_name = json_reader.GetSerializationSettingsRequests().at("file"s).AsString();
        std::ifstream in_file(file_name, std::ios::binary);

        catalogue_proto.ParseFromIstream(&in_file);
    }

    svg::Color DeserializeUnderlayerColor(const transport_catalogue_proto::Color& underlayer_color_proto) {
        if(underlayer_color_proto.has_rgb()) {
            svg::Rgb rgb;
            const transport_catalogue_proto::Rgb& rgb_proto = underlayer_color_proto.rgb();
            rgb.red = rgb_proto.red();
            rgb.green = rgb_proto.green();
            rgb.blue = rgb_proto.blue();
            return {svg::Rgb(rgb)};
        } else if(underlayer_color_proto.has_rgba()) {
            svg::Rgba rgba;
            const transport_catalogue_proto::Rgba& rgba_proto = underlayer_color_proto.rgba();
            rgba.red = rgba_proto.red();
            rgba.green = rgba_proto.green();
            rgba.blue = rgba_proto.blue();
            rgba.opacity = rgba_proto.opacity();
            return {svg::Rgba(rgba)};
        } else {
            return {svg::Color(underlayer_color_proto.color_string())};
        }
    }

    svg::Color DeserializeColorPalette(const transport_catalogue_proto::Color& color_palette_proto) {
        if(color_palette_proto.has_rgb()) {
            svg::Rgb rgb;
            const transport_catalogue_proto::Rgb& rgb_proto = color_palette_proto.rgb();
            rgb.red = rgb_proto.red();
            rgb.green = rgb_proto.green();
            rgb.blue = rgb_proto.blue();
            return {svg::Rgb(rgb)};

        } else if(color_palette_proto.has_rgba()) {
            svg::Rgba rgba;
            const transport_catalogue_proto::Rgba& rgba_proto = color_palette_proto.rgba();
            rgba.red = rgba_proto.red();
            rgba.green = rgba_proto.green();
            rgba.blue = rgba_proto.blue();
            rgba.opacity = rgba_proto.opacity();
            return {svg::Rgba(rgba)};

        } else {
            return {svg::Color(color_palette_proto.color_string())};
        }
    }

    void DeserializeTransportCatalogue(transport_catalogue::TransportCatalogue& transport_catalogue, transport_catalogue_proto::Catalogue& catalogue_proto) {
        std::unordered_map<uint32_t, std::string> stop_indexes;
        stop_indexes.reserve(catalogue_proto.transport_catalogue().stops_size());

        for(auto& stop_proto : catalogue_proto.transport_catalogue().stops()) {
            stop_indexes[stop_proto.id()] = stop_proto.name();
            transport_catalogue.AddStop(stop_proto.name(), {stop_proto.coordinates().lat(), stop_proto.coordinates().lng()});
        }

        for(auto& bus_proto : catalogue_proto.transport_catalogue().buses()) {
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

        for(const auto& stop_to_stop_distance_proto : catalogue_proto.transport_catalogue().stop_to_stop_distances()) {
            std::string from_stop_name = stop_indexes.at(stop_to_stop_distance_proto.from_stop_id());
            std::string to_stop_name = stop_indexes.at(stop_to_stop_distance_proto.to_stop_id());

            transport_catalogue.SetStopToStopDistances(from_stop_name, to_stop_name, stop_to_stop_distance_proto.distance());
        }
    }

    void DeserializeRenderSettings(renderer::RenderSettings& render_settings, transport_catalogue_proto::Catalogue& catalogue_proto) {
        render_settings.width = catalogue_proto.render_settings().width();
        render_settings.height = catalogue_proto.render_settings().height();
        render_settings.padding = catalogue_proto.render_settings().padding();
        render_settings.line_width = catalogue_proto.render_settings().line_width();
        render_settings.stop_radius = catalogue_proto.render_settings().stop_radius();
        render_settings.bus_label_font_size = catalogue_proto.render_settings().bus_label_font_size();

        const transport_catalogue_proto::Point& bus_label_offset_proto = catalogue_proto.render_settings().bus_label_offset();
        render_settings.bus_label_offset.x = bus_label_offset_proto.x();
        render_settings.bus_label_offset.y = bus_label_offset_proto.y();

        render_settings.stop_label_font_size = catalogue_proto.render_settings().stop_label_font_size();

        const transport_catalogue_proto::Point& stop_label_offset_proto = catalogue_proto.render_settings().stop_label_offset();
        render_settings.stop_label_offset.x = stop_label_offset_proto.x();
        render_settings.stop_label_offset.y = stop_label_offset_proto.y();

        transport_catalogue_proto::Color underlayer_color_proto = catalogue_proto.render_settings().underlayer_color();
        render_settings.underlayer_color = DeserializeUnderlayerColor(underlayer_color_proto);

        render_settings.underlayer_width = catalogue_proto.render_settings().underlayer_width();

        std::vector<svg::Color> color_palette;
        color_palette.reserve(catalogue_proto.render_settings().color_palette().size());

        std::unordered_map<uint32_t, transport_catalogue_proto::Color> color_palette_proto_indexes;
        color_palette_proto_indexes.reserve(catalogue_proto.render_settings().color_palette().size());

        for(const auto& color_proto : catalogue_proto.render_settings().color_palette()) {
            color_palette_proto_indexes[color_proto.id()] = color_proto.color();
        }

        for (uint32_t i = 0; i < color_palette_proto_indexes.size(); ++i) {
            svg::Color color = DeserializeColorPalette(color_palette_proto_indexes.at(i));
            color_palette.push_back(color);
        }
        render_settings.color_palette = std::move(color_palette);

    }

} // namespace serialization