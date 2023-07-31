#include "map_renderer.h"

namespace renderer {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    MapVisualizationSettings::MapVisualizationSettings(double w, double h, double p) 
        : max_width(w)
        , max_height(h)
        , padding(p) {
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    void MapRenderer::AddNewPointByRouteName(const std::string& route_name, const svg::Point& point, const json::Node& render_attachments) {
        using namespace std::literals;
        double line_width = render_attachments.AsDict().at("line_width"s).AsDouble();
        svg::Color color = GetCurrentColor();
        if(routes_polyline_.count(route_name) > 0) {
            routes_polyline_.at(route_name)
                .AddPoint(point)
                .SetFillColor("none"s)
                .SetStrokeColor(color)
                .SetStrokeWidth(line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            return;
        }
        bus_stops_names_.push_back(route_name);
        routes_polyline_[bus_stops_names_.back()]
            .AddPoint(point)
            .SetFillColor("none"s)
            .SetStrokeColor(color)
            .SetStrokeWidth(line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    void MapRenderer::AddNewTextForRoute(const std::string& route_name, const svg::Point& point, const json::Node& render_attachments) {
        using namespace std::literals;
        svg::Text text;
        svg::Point offset = {
            render_attachments.AsDict().at("bus_label_offset"s).AsArray().at(0).AsDouble(),
            render_attachments.AsDict().at("bus_label_offset"s).AsArray().at(1).AsDouble()
        };
        uint32_t font_size = static_cast<uint32_t>(render_attachments.AsDict().at("bus_label_font_size"s).AsInt());
        std::string font_family = "Verdana"s;
        std::string font_weight = "bold"s;
        svg::Color color = GetCurrentColor();

        svg::Text text_substrate = SetNewSubstrateForText(point, render_attachments);
        text_substrate.SetData(bus_stops_names_.back());
        routes_texts_[bus_stops_names_.back()].push_back(text_substrate);

        text.SetFillColor(color)
            .SetPosition(point)
            .SetOffset(offset)
            .SetFontSize(font_size)
            .SetFontFamily(font_family)
            .SetFontWeight(font_weight)
            .SetData(route_name);
        routes_texts_[bus_stops_names_.back()].push_back(text);
    }

    svg::Text MapRenderer::SetNewSubstrateForText(const svg::Point& point, const json::Node& render_attachments) {
        using namespace std::literals;
        svg::Text text_substrate;
        svg::Color fill_and_stoke_color = GetColorFromUnderlayerColorNode(render_attachments);
        double stroke_width = render_attachments.AsDict().at("underlayer_width"s).AsDouble();
        svg::Point offset = {
            render_attachments.AsDict().at("bus_label_offset"s).AsArray().at(0).AsDouble(),
            render_attachments.AsDict().at("bus_label_offset"s).AsArray().at(1).AsDouble()
        };
        uint32_t font_size = static_cast<uint32_t>(render_attachments.AsDict().at("bus_label_font_size"s).AsInt());
        svg::StrokeLineCap line_cap = svg::StrokeLineCap::ROUND;
        svg::StrokeLineJoin line_join = svg::StrokeLineJoin::ROUND;
        std::string font_family = "Verdana"s;
        std::string font_weight = "bold"s;
        text_substrate.SetFillColor(fill_and_stoke_color)
            .SetStrokeColor(fill_and_stoke_color)
            .SetStrokeWidth(stroke_width).SetStrokeLineCap(line_cap)
            .SetStrokeLineJoin(line_join)
            .SetPosition(point)
            .SetOffset(offset)
            .SetFontSize(font_size)
            .SetFontFamily(font_family)
            .SetFontWeight(font_weight);
        return text_substrate;
    }

    void MapRenderer::AddNewCircleForStop(const std::string_view stop_name, const svg::Point& point, const json::Node& render_attachments) {
        if(stops_circles_.count(stop_name) > 0) {
            return;
        } 
        using namespace std::literals;
        svg::Circle circle;
        double radius = render_attachments.AsDict().at("stop_radius"s).AsDouble();
        svg::Color color("white"s);
        circle.SetCenter(point).SetRadius(radius).SetFillColor(color);
        stops_circles_[stop_name].push_back(circle);
    }

    void MapRenderer::AddNewTextForStop(const std::string_view stop_name, const svg::Point& point, const json::Node& render_attachments) {
        if(stops_texts_.count(stop_name) > 0) {
            return;
        }
        using namespace std::literals;
        svg::Text text;
        svg::Color fill("black"s);
        svg::Point offset = {
            render_attachments.AsDict().at("stop_label_offset"s).AsArray().at(0).AsDouble(),
            render_attachments.AsDict().at("stop_label_offset"s).AsArray().at(1).AsDouble()
        };
        uint32_t font_size = static_cast<uint32_t>(render_attachments.AsDict().at("stop_label_font_size"s).AsInt());
        std::string font_family = "Verdana"s;
        std::string stop = {stop_name.data(), stop_name.size()};
        svg::Text text_substrate = SetNewSubstrateForText(point, render_attachments);
        text_substrate.SetOffset(offset)
            .SetFontSize(font_size)
            .SetFontWeight("")
            .SetData(stop);
        stops_texts_[stop_name].push_back(text_substrate);
        text.SetFillColor(fill)
            .SetPosition(point)
            .SetOffset(offset)
            .SetFontSize(font_size)
            .SetFontFamily(font_family)
            .SetData(stop);
        stops_texts_[stop_name].push_back(text);
    }

    void MapRenderer::Render(std::ostream& output) const {
        svg::Document doc;
        for(auto& [_, polyline] : routes_polyline_) {
            doc.Add(std::move(polyline));
        }
        for(auto& [_, text] : routes_texts_) {
            for(auto& t : text) {
                doc.Add(std::move(t));
            }
        }
        for(auto& [_, circle] : stops_circles_) {
            for(auto& c : circle) {
                doc.Add(std::move(c));
            }
        }
        for(auto& [_, stop_text] : stops_texts_) {
            for(auto& t : stop_text) {
                doc.Add(std::move(t));
            }
        }
        doc.Render(output);
    }

    void MapRenderer::SetPossibleColors(const std::vector<svg::Color>& colors) {
        size_t curr_color = 0;
        for(const auto& color : colors) {
            color_palette_.emplace(curr_color, color);
            ++curr_color;
        }
    }

    void MapRenderer::ChangeCurrentColor() {
        if(current_color_ == color_palette_.size() - 1) {
            current_color_ = 0;
            return;
        }
        ++current_color_;
    }

    svg::Color MapRenderer::GetCurrentColor() const {
        return color_palette_.at(current_color_);
    }

    svg::Color GetColorFromUnderlayerColorNode(const json::Node& render_attachments) {
        using namespace std::literals;
        svg::Color color;
        json::Node attachments = render_attachments.AsDict().at("underlayer_color"s);
        if(attachments.IsString()) {
            color = attachments.AsString();
        } else if (attachments.IsArray() && attachments.AsArray().size() >= 3) {
            uint8_t red_color = static_cast<uint8_t>(attachments.AsArray().at(0).AsInt());
            uint8_t green_color = static_cast<uint8_t>(attachments.AsArray().at(1).AsInt());
            uint8_t blue_color = static_cast<uint8_t>(attachments.AsArray().at(2).AsInt());
            if(attachments.AsArray().size() == 3) {
                color = svg::Rgb{red_color, green_color, blue_color};
            } else {
                double opacity = attachments.AsArray().at(3).AsDouble();
                color = svg::Rgba{red_color, green_color, blue_color, opacity};
            }
        }
        return color;
    }

    std::vector<svg::Color> GetColorFromColorPaletteNode(const json::Node& render_attachments) {
        using namespace std::literals;
        std::vector<svg::Color> result;
        result.reserve(render_attachments.AsDict().size());
        json::Node attachments = render_attachments.AsDict().at("color_palette"s);
        for(auto& node : attachments.AsArray()) {
            svg::Color current_color;
            if(node.IsString()) {
                current_color = node.AsString();
            } else if (node.IsArray() && node.AsArray().size() >= 3) {
                uint8_t red_color = static_cast<uint8_t>(node.AsArray().at(0).AsInt());
                uint8_t green_color = static_cast<uint8_t>(node.AsArray().at(1).AsInt());
                uint8_t blue_color = static_cast<uint8_t>(node.AsArray().at(2).AsInt());
                if(node.AsArray().size() == 3) {
                    current_color = svg::Rgb{red_color, green_color, blue_color};
                } else {
                    double opacity = node.AsArray().at(3).AsDouble();
                    current_color = svg::Rgba{red_color, green_color, blue_color, opacity};
                }
            }
            result.push_back(current_color);
        }
        return result;
    }
}