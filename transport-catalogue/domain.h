#pragma once
#include <string_view>
#include <vector>
#include "geo.h"

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
}