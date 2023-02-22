#pragma once
#include<string>
#include<vector>
#include<deque>
#include<unordered_map>
#include<unordered_set>

#include "transport_catalogue.h"
#include "geo.h"

using namespace transport_catalogue::detail;

namespace input_reader {
	class InputReader {
	public:
		void SetNumOfRequests(std::istream& input);
		bool IsNumOfRequestsCorrect() const;
		int GetNumOfRequests() const;
		void FillingTheQuery(std::istream& input);
		void AddInQuery(std::string line);
		void AddInStopToStopDistQuery(std::string stop, std::string line);
		std::deque<std::string> GetQuery() const;
		std::unordered_map<std::string, std::unordered_set<std::string>> GetStopToStopDistQuery() const;
	private:
		int num_of_requests_ = 0;
		std::deque<std::string> query_;
		std::unordered_map<std::string, std::unordered_set<std::string>> stop_to_stop_dist_query_;
	};
}


namespace parser {
	class Parser {

		using BusTasks = std::unordered_map<std::string, std::vector<std::string>>;
		using StopTasks = std::unordered_map<std::string, std::pair<Coordinates, std::string>>;

	public:
		Parser(std::deque<std::string> input);

		bool IsShowRequest(std::string str) const;
		bool IsBusRequest(std::string str) const;

		std::string GetRouteName(std::string str) const;
		std::string GetStopName(std::string str) const;
		std::vector<std::string> GetStopNames(std::string str) const;
		Coordinates GetStopCoordinates(std::string str) const;

		StopTasks GetAddingStopsTasks() const;
		BusTasks GetAddingBusTasks() const;
		std::deque<std::string> GetShowingTasks() const;

	private:
		StopTasks tasks_add_stops_;
		BusTasks tasks_add_buses_;
		std::deque<std::string> tasks_show_;

		void AddShowingTask(std::string str);
		void AddTaskAddingStop(std::string str, Coordinates coordinates, std::string distance_for_each_stop);
		void AddTaskAddingBus(std::string str, std::vector<std::string> stop_names);

	};
}

void ParseStopToStopString(transport_catalogue::TransportCatalogue* transport_catalogue, const std::string stop_name, std::string distance_for_each_stop);
void ProcessingCommandsByFilling(transport_catalogue::TransportCatalogue& transport_catalogue, bool& is_finished, std::istream& input_stream);