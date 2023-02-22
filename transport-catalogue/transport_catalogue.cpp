#include <iomanip>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace stat_reader;
using namespace transport_catalogue::detail;

namespace transport_catalogue {
	size_t StopPairHash::operator()(const std::pair<const Stop*, const Stop*>& p) const {
		return std::hash<const void*>()(p.first) * std::hash<const void*>()(p.second) * 1000000 + 43;
	}

	void TransportCatalogue::AddStop(std::string stop_name, detail::Coordinates coordinates) {
		if (stop_index_.count(stop_name) == 0) {
			stop_and_buses_names_.push_back(stop_name);
			stops_.push_back({ stop_and_buses_names_.back(), coordinates});
			std::string_view view_name = stops_.back().name;
			stop_index_[view_name] = &stops_.back();
		}
	}

	void TransportCatalogue::SetStopToStopDistances(const std::string stop_name, const std::string other_stop_name, std::string distance) {
		if (stop_to_stop_distances_.count({ stop_index_[other_stop_name], stop_index_[stop_name] }) == 0) {
			stop_to_stop_distances_[{ stop_index_[stop_name], stop_index_[other_stop_name] }] = std::stod(distance);
			stop_to_stop_distances_[{ stop_index_[other_stop_name], stop_index_[stop_name] }] = std::stod(distance);
		}
		else {
			stop_to_stop_distances_[{ stop_index_[stop_name], stop_index_[other_stop_name] }] = std::stod(distance);
		}
	}

	void TransportCatalogue::AddBus(std::string bus_name, const std::vector<std::string>& vect_stops, bool is_circle) {
		if (buses_index_.count(bus_name) == 0) {
			std::vector<const Stop*> stops;
			for (std::string stop : vect_stops) {
				stops.push_back(stop_index_.at(stop));
			}
			stop_and_buses_names_.push_back(bus_name);
			buses_.push_back({ stop_and_buses_names_.back(), stops, is_circle});
			std::string_view bus_name_view = buses_.back().name;
			buses_index_[bus_name_view] = &buses_.back();
			for (std::string_view stop : vect_stops) {
				std::string_view stop_name = stop_index_[stop]->name;
				buses_by_stop_[stop_name].insert(buses_.back().name);
			}
		}
	}

	transport_catalogue::BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const {
		if (buses_index_.count(name) > 0) {
			BusInfo bus_info{};
			const transport_catalogue::Bus* bus = buses_index_.at(name);
			std::vector<const transport_catalogue::Stop*> all_stops = bus->stops;
			auto stop1 = all_stops.begin();
			auto stop2 = all_stops.begin();
			++stop2;
			double road_distance = 0.0;
			double geo_road_length = 0.0;
			while (stop2 != all_stops.end()) {
				geo_road_length += ComputeDistance((*stop1)->coordinates, (*stop2)->coordinates);
				road_distance += GetStopToStopDistance(*stop1, *stop2);
				++stop1;
				++stop2;
			}

			double curvature = road_distance / geo_road_length;
			std::unordered_set<const transport_catalogue::Stop*> unique_stops(all_stops.begin(), all_stops.end());
			return bus_info = { bus->name, all_stops.size(), unique_stops.size(), static_cast<int>(road_distance),  curvature };
		}
		static BusInfo empty_bus_info{};
		empty_bus_info.name = name;
		empty_bus_info.is_empty = true;
		return empty_bus_info;
	}

	transport_catalogue::StopInfo TransportCatalogue::GetStopInfo(std::string_view name) const {
		StopInfo stop_info{};
		auto stop_buses = GetBusesByStop(name);
		std::string buses = "";
		if (!stop_buses.empty()) {
			for (auto bus : stop_buses) {
				buses += bus;
				buses += " ";
			}
		}
		return stop_info = { name, buses };
	}

	std::vector<const Stop*> TransportCatalogue::GetBusRouteByName(std::string_view name) const {
		if (buses_index_.count(name) > 0) {
			return buses_index_.at(name)->stops;
		}
		static std::vector<const Stop*> empty_stops{};
		return empty_stops;
	}

	Stop TransportCatalogue::GetStopByName(const std::string_view stop_name) const {
		if (stop_index_.count(stop_name) > 0) {
			return { stop_index_.at(stop_name)->name, stop_index_.at(stop_name)->coordinates };
		}
		static Stop empty_stop{};
		return empty_stop;
	}

	std::set<std::string_view> TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
		if (buses_by_stop_.count(stop_name)) {
			return buses_by_stop_.at(stop_name);
		}
		static std::set<std::string_view> buses{};
		return buses;
	}

	double TransportCatalogue::GetStopToStopDistance(const Stop* from, const Stop* to) const {
		return stop_to_stop_distances_.at({ from, to });
	}

	bool TransportCatalogue::IsStopExist(std::string name) const {
		if (stop_index_.count(name) > 0) {
			return true;
		}
		return false;
	}

	std::vector<const Stop*> TransportCatalogue::GetStopsByBusName(std::string name) const {
		return buses_index_.at(name)->stops;
	}
	const Bus* TransportCatalogue::GetBusByName(const std::string name) const {
		return buses_index_.at(name);
	}
}