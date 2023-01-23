#pragma once
#include<string>
#include<vector>
#include<deque>
#include<unordered_map>

#include "geo.h"

using namespace transport_catalogue::detail;

namespace input_reader {
	class InputReader {
	public:
		explicit InputReader(int num);

		void AddInQuery(std::string line);
		void ClearQuery();
		std::deque<std::string> GetQuery() const;

	private:
		std::deque<std::string> query_;
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
		std::string GetShowRequest(std::string str) const;
		Coordinates GetStopCoordinates(std::string str) const;

		StopTasks GetAddStopsTasks() const;
		BusTasks GetAddBusTasks() const;
		std::deque<std::string> GetShowTasks() const;

	private:
		StopTasks tasks_add_stops_;
		BusTasks tasks_add_buses_;
		std::deque<std::string> tasks_show_;

		void AddInTasksShow(std::string str);
		void AddInTasksAddStops(std::string str, Coordinates coordinates, std::string distance_for_each_stop);
		void AddInTasksAddBuses(std::string str, std::vector<std::string> stop_names);

	};
}