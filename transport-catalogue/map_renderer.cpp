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

    void MapRenderer::AddNewPointByRouteName(const std::string& route_name, const svg::Point& point, const json::Node& render_attachmets) {
        using namespace std::literals;
        double line_width = render_attachmets.AsMap().at("line_width"s).AsDouble();
        svg::Color color = GetColor();
        if(routes_polyline_.count(route_name) > 0) {
            routes_polyline_.at(route_name).AddPoint(point).SetFillColor("none"s).SetStrokeColor(color).SetStrokeWidth(line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            return;
        }
        bus_stops_names_.push_back(route_name);
        routes_polyline_[bus_stops_names_.back()].AddPoint(point).SetFillColor("none"s).SetStrokeColor(color).SetStrokeWidth(line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    void MapRenderer::AddNewTextForRoute(const std::string& route_name, const svg::Point& point, const json::Node& render_attachments) {
        using namespace std::literals;
        svg::Text text;
        svg::Point offset = {render_attachments.AsMap().at("bus_label_offset"s).AsArray().at(0).AsDouble(), render_attachments.AsMap().at("bus_label_offset"s).AsArray().at(1).AsDouble()};
        uint32_t font_size = static_cast<uint32_t>(render_attachments.AsMap().at("bus_label_font_size"s).AsInt());
        std::string font_family = "Verdana"s;
        std::string font_weight = "bold"s;
        svg::Color color = GetColor();

        svg::Text text_substrate = SetNewSubstrateForText(point, render_attachments);
        text_substrate.SetData(bus_stops_names_.back());
        routes_texts_[bus_stops_names_.back()].push_back(text_substrate);

        text.SetFillColor(color).SetPosition(point).SetOffset(offset).SetFontSize(font_size).SetFontFamily(font_family).SetFontWeight(font_weight).SetData(route_name);
        routes_texts_[bus_stops_names_.back()].push_back(text);
    }

    svg::Text MapRenderer::SetNewSubstrateForText(const svg::Point& point, const json::Node& render_attachments) {
        using namespace std::literals;
        svg::Text text_substrate;
        svg::Color fill_and_stoke_color = GetColorFromNode(render_attachments);
        double stroke_width = render_attachments.AsMap().at("underlayer_width"s).AsDouble();
        svg::Point offset = {render_attachments.AsMap().at("bus_label_offset"s).AsArray().at(0).AsDouble(), render_attachments.AsMap().at("bus_label_offset"s).AsArray().at(1).AsDouble()};
        uint32_t font_size = static_cast<uint32_t>(render_attachments.AsMap().at("bus_label_font_size"s).AsInt());
        svg::StrokeLineCap line_cap = svg::StrokeLineCap::ROUND;
        svg::StrokeLineJoin line_join = svg::StrokeLineJoin::ROUND;
        std::string font_family = "Verdana"s;
        std::string font_weight = "bold"s;
        text_substrate.SetFillColor(fill_and_stoke_color).SetStrokeColor(fill_and_stoke_color).SetStrokeWidth(stroke_width).SetStrokeLineCap(line_cap).SetStrokeLineJoin(line_join).SetPosition(point).SetOffset(offset).SetFontSize(font_size).SetFontFamily(font_family).SetFontWeight(font_weight);
        return text_substrate;
    }

    void MapRenderer::AddNewCircleForStop(const std::string_view stop_name, const svg::Point& point, const json::Node& render_attachments) {
        if(stops_circles_.count(stop_name) > 0) {
            return;
        } 
        using namespace std::literals;
        svg::Circle circle;
        double radius = render_attachments.AsMap().at("stop_radius"s).AsDouble();
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
        svg::Point offset = {render_attachments.AsMap().at("stop_label_offset"s).AsArray().at(0).AsDouble(), render_attachments.AsMap().at("stop_label_offset"s).AsArray().at(1).AsDouble()};
        uint32_t font_size = static_cast<uint32_t>(render_attachments.AsMap().at("stop_label_font_size"s).AsInt());
        std::string font_family = "Verdana"s;
        std::string stop = {stop_name.data(), stop_name.size()};
        svg::Text text_substrate = SetNewSubstrateForText(point, render_attachments);
        text_substrate.SetOffset(offset);
        text_substrate.SetFontSize(font_size);
        text_substrate.SetFontWeight("");
        text_substrate.SetData(stop);
        stops_texts_[stop_name].push_back(text_substrate);
        text.SetFillColor(fill).SetPosition(point).SetOffset(offset).SetFontSize(font_size).SetFontFamily(font_family).SetData(stop);
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

    void MapRenderer::SetPossibleColors(const json::Array& colors) {
        size_t curr_color = 0;
        for(const auto& color : colors) {
            possible_colors_for_route_lines_.emplace(curr_color, color);
            ++curr_color;
        }
    }

    void MapRenderer::ChangeCurrentColor() {
        if(current_color_ == possible_colors_for_route_lines_.size() - 1) {
            current_color_ = 0;
            return;
        }
        ++current_color_;
    }

    json::Node MapRenderer::GetCurrentColor() const {
        return possible_colors_for_route_lines_.at(current_color_);
    }

    svg::Color MapRenderer::GetColorFromNode(const json::Node& render_attachments) const {
        using namespace std::literals;
        svg::Color color;
        if(render_attachments.AsMap().at("underlayer_color"s).IsString()) {
            color = render_attachments.AsMap().at("underlayer_color"s).AsString();
        } else {
            uint8_t r, g, b;
            r = static_cast<uint8_t>(render_attachments.AsMap().at("underlayer_color"s).AsArray().at(0).AsInt());
            g = static_cast<uint8_t>(render_attachments.AsMap().at("underlayer_color"s).AsArray().at(1).AsInt());
            b = static_cast<uint8_t>(render_attachments.AsMap().at("underlayer_color"s).AsArray().at(2).AsInt());
            if(render_attachments.AsMap().at("underlayer_color"s).AsArray().size() == 3) {
                color = svg::Rgb{r, g, b};
            } else {
                double o = render_attachments.AsMap().at("underlayer_color"s).AsArray().at(3).AsDouble();
                color = svg::Rgba{r, g, b, o};
            }
        }
        return color;
    }

    svg::Color MapRenderer::GetColor() const {
        svg::Color color;
        if(possible_colors_for_route_lines_.at(current_color_).IsString()) {
            color = possible_colors_for_route_lines_.at(current_color_).AsString();
        } else {
            uint8_t r, g, b;
            r = static_cast<uint8_t>(possible_colors_for_route_lines_.at(current_color_).AsArray().at(0).AsInt());
            g = static_cast<uint8_t>(possible_colors_for_route_lines_.at(current_color_).AsArray().at(1).AsInt());
            b = static_cast<uint8_t>(possible_colors_for_route_lines_.at(current_color_).AsArray().at(2).AsInt());
            if(possible_colors_for_route_lines_.at(current_color_).AsArray().size() == 3) {
                color = svg::Rgb{r, g, b};
            } else {
                double o = possible_colors_for_route_lines_.at(current_color_).AsArray().at(3).AsDouble();
                color = svg::Rgba{r, g, b, o};
            }
        }
        return color;
    }
}