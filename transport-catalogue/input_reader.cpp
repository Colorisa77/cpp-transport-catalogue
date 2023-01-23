#include <string>
#include <iostream>

#include "input_reader.h"

using namespace transport_catalogue::detail;


namespace input_reader {
    void InputReader::AddInQuery(std::string line) {
        query_.push_back(std::move(line));
    }

    std::deque<std::string> InputReader::GetQuery() const {
        return query_;
    }

    void InputReader::ClearQuery() {
        query_.clear();
    }

    InputReader::InputReader(const int num) {
        int curr_num = 0;
        while (curr_num != num) {
            std::string line = "";
            std::getline(std::cin, line);
            if (line.empty()) {
                continue;
            }
            AddInQuery(std::move(line));
            ++curr_num;
        }
    }
}

namespace parser {
    using BusTasks = std::unordered_map<std::string, std::vector<std::string>>;
    using StopTasks = std::unordered_map<std::string, std::pair<Coordinates, std::string>>;

    Parser::Parser(std::deque<std::string> input) {
        while (!input.empty()) {
            std::string str_query = input.front();
            if (IsShowRequest(str_query)) {
                AddInTasksShow(str_query);
            }
            else if (IsBusRequest(str_query)) {
                std::string route_name = GetRouteName(str_query);
                std::vector<std::string> stop_names = GetStopNames(str_query);
                AddInTasksAddBuses(route_name, stop_names);
            }
            else {
                std::string stop_name = GetStopName(str_query);
                Coordinates coords = GetStopCoordinates(str_query);
                size_t comma_pos = str_query.find(",");
                std::string distanse_for_each_stop = str_query.substr(comma_pos + 1);
                comma_pos = distanse_for_each_stop.find(",");
                if (comma_pos == std::string::npos) {
                    AddInTasksAddStops(stop_name, coords, "");
                }
                else {
                    distanse_for_each_stop = distanse_for_each_stop.substr(comma_pos + 2);
                    AddInTasksAddStops(stop_name, coords, distanse_for_each_stop);
                }
            }
            input.pop_front();
        }
    }

    bool Parser::IsShowRequest(std::string str) const {
        return str.find(':') == std::string::npos;
    }

    bool Parser::IsBusRequest(std::string str) const {
        return str.substr(0, 3) == "Bus";
    }

    std::string Parser::GetRouteName(std::string str) const {
        size_t first_space_pos = str.find_first_of(" ");
        size_t pos_colon = str.find_first_of(":");
        return str.substr(first_space_pos + 1, pos_colon - first_space_pos - 1);
    }

    std::string Parser::GetStopName(std::string str) const {
        size_t first_space_pos = str.find_first_of(" ");
        size_t pos_colon = str.find_first_of(":");
        return str.substr(first_space_pos + 1, pos_colon - first_space_pos - 1);
    }

    std::vector<std::string> Parser::GetStopNames(std::string str) const {
        size_t pos_colon = str.find_first_of(":");
        std::string stop_names_query = str.substr(pos_colon + 2);
        std::vector<std::string> stop_names;
        std::deque<std::string> stops_continue;
        bool is_minus = false;
        if (stop_names_query.find('-') != std::string::npos) {
            is_minus = true;
        }
        char symbol;
        is_minus == true ? symbol = '-' : symbol = '>';
        size_t symb = stop_names_query.find_first_of(symbol);
        while (symb != std::string::npos) {
            stop_names.push_back(stop_names_query.substr(0, symb - 1));
            if (symbol == '-') {
                stops_continue.push_front(stop_names.back());
            }
            stop_names_query = stop_names_query.substr(symb + 2);
            symb = stop_names_query.find_first_of(symbol);
        }
        stop_names.push_back(stop_names_query);
        if (symbol == '-') {
            stops_continue.push_front(stop_names.back());
        }
        if (is_minus == true) {
            stops_continue.pop_front();
            while (!stops_continue.empty()) {
                stop_names.push_back(stops_continue.front());
                stops_continue.pop_front();
            }
        }
        return stop_names;
    }

    std::deque<std::string> Parser::GetShowTasks() const {
        return tasks_show_;
    }

    std::string Parser::GetShowRequest(std::string str) const {
        size_t first_space_pos = str.find_first_of(" ");
        return str.substr(first_space_pos + 1, str.size());
    }

    Coordinates Parser::GetStopCoordinates(std::string str) const {
        size_t pos_colon = str.find_first_of(":");
        std::string coordinates_str = str.substr(pos_colon + 1);
        size_t comma_pos = coordinates_str.find(',');
        std::string latitude_str = coordinates_str.substr(1, comma_pos - 1);
        coordinates_str = coordinates_str.substr(comma_pos + 1);
        std::string longitude_str;
        comma_pos = coordinates_str.find(',');
        if (comma_pos == std::string::npos) {
            longitude_str = coordinates_str.substr(1, coordinates_str.size());
        }
        else {
            longitude_str = coordinates_str.substr(1, comma_pos - 1);
        }
        double latitude = std::stod(latitude_str);
        double longitude = std::stod(longitude_str);
        return { latitude, longitude };
    }

    StopTasks Parser::GetAddStopsTasks() const {
        return tasks_add_stops_;
    }

    BusTasks Parser::GetAddBusTasks() const {
        return tasks_add_buses_;
    }

    void Parser::AddInTasksShow(std::string str) {
        tasks_show_.push_back(std::move(str));
    }

    void Parser::AddInTasksAddStops(std::string str, Coordinates coordinates, std::string distance_for_each_stop) {
        tasks_add_stops_[str] = { coordinates, distance_for_each_stop };
    }

    void Parser::AddInTasksAddBuses(std::string str, std::vector<std::string> stop_names) {
        tasks_add_buses_[str] = std::move(stop_names);
    }
}
