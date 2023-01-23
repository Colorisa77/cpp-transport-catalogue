#include <iostream>
#include <string>

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

int main()
{
    int count;
    transport_catalogue::TransportCatalogue transport_catalogue;
    while (true) {
        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) {
            break;
        }
        try {
            count = std::stoi(input);
        }
        catch (std::invalid_argument&) {
            continue;
        }
        input_reader::InputReader input_queue(count);
        parser::Parser parser(input_queue.GetQuery());
        transport_catalogue.DoAddStopTask(std::move(parser.GetAddStopsTasks()));
        transport_catalogue.DoAddBusTask(std::move(parser.GetAddBusTasks()));
        stat_reader::StatReader stat_reader(transport_catalogue.DoShowTask(parser.GetShowTasks()));
        stat_reader.DoShowRequest();
        input_queue.ClearQuery();
    }
}