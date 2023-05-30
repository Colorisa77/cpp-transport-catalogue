#pragma once
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <set>
#include "domain.h"

namespace transport_catalogue {

	struct StopPairHash {
		size_t operator()(const std::pair<const Stop*, const Stop*>& p) const;
	};

	class TransportCatalogue final {
	public:

		BusInfo GetBusInfo(std::string_view name) const;
		StopInfo GetStopInfo(std::string_view name) const;
		std::vector<const Stop*> GetBusRouteByName(const std::string_view route_name) const;
		Stop GetStopByName(const std::string_view stop_name) const;
		std::vector<geo::Coordinates> GetCoordinatesFromStopsWithCoordinates() const;

		std::vector<std::string_view> GetAllBusesFromCatalogue() const;
		std::vector<std::string_view> GetAllStopsFromCatalogue() const;

		void AddStop(std::string stop_name, geo::Coordinates coordinates);
		void AddBus(std::string bus_name, std::vector<std::string>& vect_stops, bool is_circle);

		std::set<std::string_view> GetBusesNamesByStop(std::string_view stop_name) const;
		const Bus* GetBusByName(const std::string name) const;

		double GetStopToStopDistance(const Stop* from, const Stop* to) const;
		double GetStopToStopDistanceByName(const std::string_view from, const std::string_view to) const;
		
		std::vector<const Stop*> GetStopsByBusName(std::string name) const;
		
		bool IsStopExist(std::string) const;
		
		void SetStopToStopDistances(const std::string stop_name, const std::string other_stop_name, double distance);


	private:
		std::deque<Bus> buses_;
		std::deque<Stop> stops_;
		std::deque<std::string> stop_and_buses_names_;
		std::vector<geo::Coordinates> stops_with_routes_;
		std::unordered_map<std::string_view, const Bus*> buses_index_;
		std::unordered_map<std::string_view, const Stop*> stop_index_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_by_stop_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopPairHash> stop_to_stop_distances_;
	};
}