#pragma once
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>
#include <string_view>
#include <deque>
#include <set>
#include "geo.h"

namespace transport_catalogue {
	struct BusInfo {
		std::string name;
		size_t count_all_stops;
		size_t count_unique_stops;
		std::string route_length;
	};

	struct Stop {
		std::string name;
		detail::Coordinates coordinates;
	};

	struct Bus {
		std::string name;
		std::vector<const Stop*> stops;
		std::unordered_set<const Stop*> unique_stops;
		bool is_circle;
		BusInfo bus_info;
	};

	struct StopPairHash {
		size_t operator()(const std::pair<const Stop*, const Stop*>& p) const;
	};


	class TransportCatalogue {
	public:

		BusInfo GetBusInfo(std::string_view name) const;
		std::vector<const Stop*> GetBusRouteByName(const std::string_view route_name) const;
		Stop GetStopByName(const std::string_view stop_name) const;

		void DoAddStopTask(std::unordered_map<std::string, std::pair<detail::Coordinates, std::string>> add_stop_task);
		void DoAddBusTask(std::unordered_map<std::string, std::vector<std::string>> add_bus_task);
		std::deque<std::string> DoShowTask(std::deque<std::string> show_route) const;

		std::set<std::string_view> GetBusesByStop(std::string_view stop_name) const;

		double GetStopToStopDistance(const Stop* stop1, const Stop* stop2) const;

		void AddStopsByBus(const Bus* bus, std::vector<std::string> stops);
		void AddStopToStopDistance(std::vector<std::pair<std::string_view, std::string>>&);


	private:
		std::deque<Bus> buses_;
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, const Bus*> buses_index_;
		std::unordered_map<std::string_view, const Stop*> stop_index_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_by_stop_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopPairHash> stop_to_stop_distances_;
	};
}
