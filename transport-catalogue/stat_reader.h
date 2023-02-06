#pragma once
#include <string>
#include <deque>

#include "transport_catalogue.h"

namespace stat_reader {

	std::string RouteLengthFormatting(std::string route_length);

	void StatReader(transport_catalogue::TransportCatalogue& transport_catalogue, std::ostream& output);
	void PrintBusInfo(transport_catalogue::BusInfo& bus_info, std::ostream& output);
	void PrintStopInfo(transport_catalogue::StopInfo& stop_info, std::ostream& output);
}