#include <iostream>
#include <string>

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

int main()
{
    transport_catalogue::TransportCatalogue transport_catalogue;
    bool is_finished = false;
    while (!is_finished) {
        ProcessingCommandsByFilling(transport_catalogue, is_finished, std::cin);
        if (is_finished == true) {
            break;
        }
    }
}