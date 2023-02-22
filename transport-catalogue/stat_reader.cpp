#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "stat_reader.h"
#include "transport_catalogue.h"
#include "geo.h"


namespace stat_reader {

	void StatReader(transport_catalogue::TransportCatalogue& transport_catalogue, std::string request, std::ostream& output) {
		if (request.empty()) {
			return;
		}
		std::string type_request = request.substr(0, request.find_first_of(' '));
		std::string name = request.substr(request.find_first_of(' ') + 1, request.size());
		if (type_request == "Bus") {
			PrintBusInfo(transport_catalogue, name, output);
		}
		else {
			PrintStopInfo(transport_catalogue, name, output);
		}
	}

	std::string RouteLengthFormatting(std::string route_length) {
		size_t decimal_pos = route_length.find(".");
		std::ostringstream result;
		if (route_length[decimal_pos + 1] == '0')
		{
			result << std::fixed << std::setprecision(2) << route_length;
		}
		else {
			result << std::fixed << std::setprecision(1) << route_length;
		}
		return result.str();
	}

	void PrintBusInfo(transport_catalogue::TransportCatalogue& transport_catalogue, std::string name, std::ostream& output) {
		std::string type_request = "Bus";
		transport_catalogue::BusInfo bus_info = transport_catalogue.GetBusInfo(name);
		if (bus_info.is_empty == false) {
			output << type_request << " " << bus_info.name << ": " << bus_info.count_all_stops << " stops on route, " << bus_info.count_unique_stops << " unique stops, " << bus_info.route_length << " route length, " << bus_info.curvature << " curvature\n";
			return;
		}
		output << type_request << " " << bus_info.name << ": not found\n";
	}

	void PrintStopInfo(transport_catalogue::TransportCatalogue& transport_catalogue, std::string name, std::ostream& output) {
		std::string type_request = "Stop";
		transport_catalogue::StopInfo stop_info = transport_catalogue.GetStopInfo(name);
		if (stop_info.is_empty == false) {
			if (stop_info.stop_buses != "") {
				output << type_request << " " << stop_info.name << ": buses " << stop_info.stop_buses << "\n";
				return;
			}
			output << type_request << " " << stop_info.name << ": no buses\n";
			return;
		}
		output << type_request << " " << stop_info.name << ": not found\n";
	}
}