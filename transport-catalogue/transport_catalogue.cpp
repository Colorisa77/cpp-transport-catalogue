#include <iomanip>
#include <stdexcept>
#include <sstream>

#include "transport_catalogue.h"
using namespace transport_catalogue::detail;

namespace transport_catalogue {
	size_t StopPairHash::operator()(const std::pair<const Stop*, const Stop*>& p) const {
		return std::hash<const void*>()(p.first) * std::hash<const void*>()(p.second) * 1000000 + 43;
	}

	void TransportCatalogue::DoAddStopTask(std::unordered_map<std::string, std::pair<Coordinates, std::string>> add_stop_task) {
		if (add_stop_task.empty()) {
			return;
		}

		std::vector<std::pair<std::string_view, std::string>> stop_to_stop_request;
		stop_to_stop_request.reserve(add_stop_task.size());
		for (auto& [name, coordinates_and_dist] : add_stop_task) {
			if (stop_index_.count(name) == 0) {
				stops_.push_back({ name, coordinates_and_dist.first });
				std::string_view view_name = stops_.back().name;
				stop_index_[view_name] = &stops_.back();
				std::string dist_and_stops = coordinates_and_dist.second;
				if (dist_and_stops != "") {
					stop_to_stop_request.push_back({ name, dist_and_stops });
				}
			}
			else {
				throw std::invalid_argument("Stop " + name + " already exist\n");
			}
		}
		AddStopToStopDistance(stop_to_stop_request);
	}

	void TransportCatalogue::AddStopToStopDistance(std::vector<std::pair<std::string_view, std::string>>& stop_to_stop_request) {
		for (auto& [stop_name, request] : stop_to_stop_request) {
			if (request == "") {
				continue;
			}
			size_t comma_pos = request.find(',');
			std::string distance;
			size_t m = request.find('m');
			std::string other_stop_name;
			if (comma_pos == std::string::npos && request.find('m') == std::string::npos) {
				continue;
			}
			while (comma_pos != std::string::npos) {
				distance = request.substr(0, m);
				request = request.substr(m + 5);
				other_stop_name = request.substr(0, request.find(','));
				request = request.substr(request.find(',') + 2);
				if (stop_to_stop_distances_.count({ stop_index_[other_stop_name], stop_index_[stop_name] }) == 0) {
					stop_to_stop_distances_[{ stop_index_[stop_name], stop_index_[other_stop_name] }] = std::stod(distance);
					stop_to_stop_distances_[{ stop_index_[other_stop_name], stop_index_[stop_name] }] = std::stod(distance);
				}
				else {
					stop_to_stop_distances_[{ stop_index_[stop_name], stop_index_[other_stop_name] }] = std::stod(distance);
				}
				comma_pos = request.find(',');
				m = request.find('m');
			}
			m = request.find('m');
			distance = request.substr(0, m);
			request = request.substr(m + 5);
			other_stop_name = request;
			if (stop_to_stop_distances_.count({ stop_index_[other_stop_name], stop_index_[stop_name] }) == 0) {
				stop_to_stop_distances_[{ stop_index_[stop_name], stop_index_[other_stop_name] }] = std::stod(distance);
				stop_to_stop_distances_[{ stop_index_[other_stop_name], stop_index_[stop_name] }] = std::stod(distance);
			}
			else {
				stop_to_stop_distances_[{ stop_index_[stop_name], stop_index_[other_stop_name] }] = std::stod(distance);
			}
		}
	}

	void TransportCatalogue::DoAddBusTask(std::unordered_map<std::string, std::vector<std::string>> add_bus_task) {
		if (add_bus_task.empty()) {
			return;
		}
		for (const auto& [name, vect_stops] : add_bus_task) {
			bool is_circle = false;
			if (vect_stops.front() == vect_stops.back()) {
				is_circle = true;
			}
			if (buses_index_.count(name) == 0) {
				std::vector<const Stop*> stops;
				std::unordered_set<const Stop*> unique_stops;
				double route_length = 0.0;
				Coordinates curr_coordinates{};
				bool is_first = true;
				for (std::string stop_name : vect_stops) {
					std::string_view view_name = stop_name;
					const Stop* stop = stop_index_[view_name];
					stops.push_back(stop);
					unique_stops.insert(stop);
					if (is_first == true) {
						curr_coordinates = stop->coordinates;
						is_first = false;
						continue;
					}
					route_length += ComputeDistance(curr_coordinates, stop->coordinates);
					curr_coordinates = stop->coordinates;
				}
				std::string r_length = std::to_string(route_length);
				size_t decimal_pos = r_length.find(".");
				std::ostringstream ss;
				if (r_length[decimal_pos + 1] == '0')
				{
					ss << std::fixed << std::setprecision(2) << route_length;
				}
				else {
					ss << std::fixed << std::setprecision(1) << route_length;
				}
				std::string result_length = ss.str();
				buses_.push_back({ name, stops, unique_stops, is_circle, {name, stops.size(), unique_stops.size(), result_length} });
				std::string_view bus_name_view = buses_.back().name;
				buses_index_[bus_name_view] = &buses_.back();
				AddStopsByBus(&buses_.back(), vect_stops);
			}
			else {
				throw std::invalid_argument("Bus " + name + " already exist\n");
			}
		}
	}

	void TransportCatalogue::AddStopsByBus(const Bus* bus, std::vector<std::string> stops) {
		for (std::string_view stop : stops) {
			std::string_view stop_name = stop_index_[stop]->name;
			buses_by_stop_[stop_name].insert(bus->name);
		}
	}

	BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const {
		if (buses_index_.count(name) > 0) {
			return buses_index_.at(name)->bus_info;
		}
		static BusInfo empty_bus_info{};
		return empty_bus_info;
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
		return buses_by_stop_.at(stop_name);
	}

	double TransportCatalogue::GetStopToStopDistance(const Stop* stop1, const Stop* stop2) const {
		return stop_to_stop_distances_.at({ stop1, stop2 });
	}


	std::deque<std::string> TransportCatalogue::DoShowTask(std::deque<std::string> show_route) const {
		if (show_route.empty()) {
			return {};
		}
		std::deque<std::string> result;
		for (std::string request : show_route) {
			std::string type_request = request.substr(0, request.find_first_of(' '));
			std::string result_text;
			std::string name = request.substr(request.find_first_of(' ') + 1, request.size());
			if (type_request == "Bus") {
				if (buses_index_.count(name) > 0) {
					BusInfo bus_info = GetBusInfo(name);
					std::vector<const Stop*> all_stops = buses_index_.at(name)->stops;
					auto stop1 = all_stops.begin();
					auto stop2 = all_stops.begin();
					++stop2;
					double road_distance = 0;
					while (stop2 != all_stops.end()) {
						road_distance += GetStopToStopDistance(*stop1, *stop2);
						++stop1;
						++stop2;
					}
					std::string curvature = std::to_string(road_distance / std::stod(bus_info.route_length));
					result_text = type_request + " " + bus_info.name + ": " + std::to_string(bus_info.count_all_stops) + " stops on route, " + std::to_string(bus_info.count_unique_stops) + " unique stops, " + std::to_string(static_cast<int>(road_distance)) + " route length, " + std::to_string(road_distance / std::stod(bus_info.route_length)) + " curvature";
				}
				else {
					result_text = type_request + " " + name + ": not found";
				}
			}
			else {
				if (stop_index_.count(name) > 0) {
					std::string stop_buses;
					if (buses_by_stop_.count(name) > 0) {
						for (std::string_view buses : GetBusesByStop(name)) {
							stop_buses += buses;
							stop_buses += " ";
						}
						result_text = type_request + " " + name + ": buses " + stop_buses;
					}
					else {
						result_text = type_request + " " + name + ": no buses";
					}
				}
				else {
					result_text = type_request + " " + name + ": not found";
				}
			}
			result.push_back(std::move(result_text));
		}
		return result;
	}
}