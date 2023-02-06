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
		std::string_view name;
		size_t count_all_stops;
		size_t count_unique_stops;
		int route_length;
		double curvature;
		bool is_empty = false;
	};

	struct StopInfo {
		std::string_view name;
		std::string stop_buses;
		bool is_empty = false;
	};

	struct Stop {
		std::string_view name;
		detail::Coordinates coordinates;
	};

	struct Bus {
		std::string_view name;
		std::vector<const Stop*> stops;
		bool is_circle;
	};

	struct StopPairHash {
		size_t operator()(const std::pair<const Stop*, const Stop*>& p) const;
	};


	class TransportCatalogue {
	public:

		BusInfo GetBusInfo(std::string_view name) const;
		StopInfo GetStopInfo(std::string_view name) const;
		std::vector<const Stop*> GetBusRouteByName(const std::string_view route_name) const;
		Stop GetStopByName(const std::string_view stop_name) const;

		void AddStop(std::string stop_name, detail::Coordinates coordinates, std::string distance_for_each_stop);
		void AddBus(std::string bus_name, std::vector<std::string>& vect_stops, bool is_circle);

		std::unordered_map<std::string, std::string> GetStopToStopDistanceTasks() const;

		void SetShowTasks(std::deque<std::string> task_show);
		std::deque<std::string> GetShowTasks() const;
		void ClearShowTasks();

		std::set<std::string_view> GetBusesByStop(std::string_view stop_name) const;
		const Bus* GetBusByName(const std::string name) const;

		double GetStopToStopDistance(const Stop* from, const Stop* to) const;
		std::vector<const Stop*> GetStopsByBusName(std::string name) const;
		
		void AddStopToStopDistances(const std::string stop_name, const std::string other_stop_name, std::string distance);

	private:
		std::deque<Bus> buses_;
		std::deque<Stop> stops_;
		std::deque<std::string> queries_;
		std::unordered_map<std::string, std::string> tasks_stop_to_stop_distance_;
		std::deque<std::string> tasks_show_;
		std::unordered_map<std::string_view, const Bus*> buses_index_;
		std::unordered_map<std::string_view, const Stop*> stop_index_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_by_stop_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopPairHash> stop_to_stop_distances_;
	};
}
