#include <string>
#include <iostream>

#include "input_reader.h"

using namespace transport_catalogue::detail;


namespace input_reader {

    void InputReader::SetNumOfRequests(std::istream& input) {
        std::string input_str;
        std::getline(input, input_str);
        if (input_str.empty()) {
            return;
        }
        try {
            num_of_requests_ = std::stoi(input_str);
        }
        catch (std::invalid_argument&) {
            return;
        }
    }

    bool InputReader::IsNumOfRequestsCorrect() const {
        if (num_of_requests_ <= 0) {
            return false;
        }
        return true;
    }

    int InputReader::GetNumOfRequests() const {
        return num_of_requests_;
    }

    void InputReader::FillingTheQuery(std::istream& input) {
    std::string line = "";
    std::getline(input, line);
    if (line.empty()) {
        return;
    }
    AddInQuery(std::move(line));
    }

    void InputReader::AddInQuery(std::string line) {
        query_.push_back(std::move(line));
    }

    std::deque<std::string> InputReader::GetQuery() const {
        return query_;
    }

    void InputReader::ClearQuery() {
        query_.clear();
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

void ParseStopToStopString(transport_catalogue::TransportCatalogue* transport_catalogue, const std::string stop_name, std::string distance_for_each_stop) {
    size_t comma_pos = distance_for_each_stop.find(',');
    std::string distance;
    size_t m = distance_for_each_stop.find('m');
    std::string other_stop_name;
    if (comma_pos == std::string::npos && distance_for_each_stop.find('m') == std::string::npos) {
        return;
    }
    while (comma_pos != std::string::npos) {
        distance = distance_for_each_stop.substr(0, m);
        distance_for_each_stop = distance_for_each_stop.substr(m + 5);
        other_stop_name = distance_for_each_stop.substr(0, distance_for_each_stop.find(','));
        distance_for_each_stop = distance_for_each_stop.substr(distance_for_each_stop.find(',') + 2);
        transport_catalogue->AddStopToStopDistances(stop_name, other_stop_name, distance);
        comma_pos = distance_for_each_stop.find(',');
        m = distance_for_each_stop.find('m');
    }
    m = distance_for_each_stop.find('m');
    distance = distance_for_each_stop.substr(0, m);
    distance_for_each_stop = distance_for_each_stop.substr(m + 5);
    other_stop_name = distance_for_each_stop;
    transport_catalogue->AddStopToStopDistances(stop_name, other_stop_name, distance);
}

void ProcessingCommandsByFilling(transport_catalogue::TransportCatalogue& transport_catalogue, bool& is_finished, std::istream& input_stream) {
    input_reader::InputReader input_queue;
    input_queue.SetNumOfRequests(std::cin);
    if (!input_queue.IsNumOfRequestsCorrect()) {
        is_finished = true;
        return;
    }
    int currnet_request = 0;
    while (currnet_request != input_queue.GetNumOfRequests()) {
        input_queue.FillingTheQuery(std::cin);
        ++currnet_request;
    }
    parser::Parser parser(input_queue.GetQuery());
    for (auto& [name, coord_and_dist] : parser.GetAddStopsTasks()) {
        transport_catalogue.AddStop(name, coord_and_dist.first, coord_and_dist.second);
    }

    for (auto& [stop_name, dist_to_other_stop] : transport_catalogue.GetStopToStopDistanceTasks()) {
        ParseStopToStopString(&transport_catalogue, stop_name, dist_to_other_stop);
    }

    for (auto& [name, vect_stops] : parser.GetAddBusTasks()) {
        bool is_circle;
        vect_stops.front() == vect_stops.back() ? is_circle = true : is_circle = false;
        transport_catalogue.AddBus(name, vect_stops, is_circle);
    }
    transport_catalogue.SetShowTasks(std::move(parser.GetShowTasks()));
    input_queue.ClearQuery();
}