#pragma once
#include <string>
#include <deque>

#include "transport_catalogue.h"

namespace stat_reader {

	std::string RouteLengthFormatting(std::string route_length);

	void StatReader(transport_catalogue::TransportCatalogue& transport_catalogue, std::string request, std::ostream& output);
	void PrintBusInfo(transport_catalogue::TransportCatalogue& transport_catalogue, std::string name, std::ostream& output);
	void PrintStopInfo(transport_catalogue::TransportCatalogue& transport_catalogue, std::string name, std::ostream& output);
}