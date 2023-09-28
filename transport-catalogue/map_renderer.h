#pragma once
#include "geo.h"
#include "domain.h"
#include "svg.h"
#include "json.h"

#include <unordered_map>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

namespace renderer {

    struct RenderSettings {
        double width;
        double height;
        double padding;
        double line_width;
        double stop_radius;
        uint32_t bus_label_font_size;
        svg::Point bus_label_offset;
        uint32_t stop_label_font_size;
        svg::Point stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width;
        std::vector<svg::Color> color_palette;
    };

    struct MapVisualizationSettings {
        double max_width;
        double max_height;
        double padding;
        MapVisualizationSettings(double max_width, double max_height, double padding);
    };

    bool IsZero(double value);
    inline const double EPSILON = 1e-6;

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
            : padding_(padding)
        {
            if (points_begin == points_end) {
                return;
            }

            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class MapRenderer {
    public:
        void AddNewPointByRouteName(const std::string& route_name, const svg::Point& point, const RenderSettings& render_settings);
        void AddNewTextForRoute(const std::string& route_name, const svg::Point& point, const RenderSettings& render_settings);
        void AddNewCircleForStop(const std::string_view stop_name, const svg::Point& point, const RenderSettings& render_settings);
        void AddNewTextForStop(const std::string_view stop_name, const svg::Point& point, const RenderSettings& render_settings);

        svg::Text SetNewSubstrateForText(const svg::Point& point, const RenderSettings& render_settings);

        void Render(std::ostream& output) const;

        void SetPossibleColors(const std::vector<svg::Color>& colors);

        void ChangeCurrentColor();

        svg::Color GetCurrentColor() const;

    private:
        size_t current_color_ = 0;
        std::unordered_map<size_t, svg::Color> color_palette_;
        std::map<std::string_view, svg::Polyline> routes_polyline_;
        std::map<std::string_view, std::deque<svg::Text>> routes_texts_;
        std::map<std::string_view, std::deque<svg::Circle>> stops_circles_;
        std::map<std::string_view, std::deque<svg::Text>> stops_texts_;
        std::deque<std::string> bus_stops_names_;
    };

    svg::Color GetColorFromUnderlayerColorNode(const json::Node& render_attachments);
    std::vector<svg::Color> GetColorFromColorPaletteNode(const json::Node& render_attachments);
}