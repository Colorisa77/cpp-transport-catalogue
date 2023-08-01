#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <optional>
#include "geo.h"
#include "svg.h"

namespace transport_catalogue {

	struct BusInfo {
		std::string_view name;
		size_t count_all_stops;
		size_t count_unique_stops;
		int route_length;
		double curvature;
		bool is_empty = true;
	};

	struct StopInfo {
		std::string_view name;
		std::vector<std::string_view> stop_buses;
		bool is_empty = true;
	};

	struct Stop {
		std::string_view name;
		geo::Coordinates coordinates;
	};

	struct Bus {
		std::string_view name;
		std::vector<const Stop*> stops;
		bool is_circle;
	};

	struct VertexToVertex {
		std::string type;
		std::string_view bus_or_stop_name;
		std::optional<int> span;
		double time;
	};

	struct RouteInfo {
		double total_time;
		std::deque<VertexToVertex> list_of_items;
	};
}