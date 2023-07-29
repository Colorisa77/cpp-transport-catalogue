#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

/*void QueryProcessing(transport_catalogue::TransportCatalogue& transport_catalogue, std::istream& input, std::ostream& output) {
    router::TransportRouter router;
    renderer::MapRenderer map_render;
    request_handler::RequestHandler request_handler(transport_catalogue, router, map_render);
    json_reader::SequentialRequestProcessing(
        transport_catalogue, 
        router, 
        map_render, 
        input, 
        output, 
        request_handler
    );
}

int main() {
    transport_catalogue::TransportCatalogue transport_catalogue;
    QueryProcessing(transport_catalogue, std::cin, std::cout);
}*/

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    json_reader::JsonReader json_reader(std::cin);

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        // make base here
        transport_catalogue::TransportCatalogue transport_catalogue;
        json_reader::FillingTransportCatalogue(transport_catalogue, json_reader);
        serialization::SerializeTransportCatalogue(transport_catalogue, json_reader);


    } else if (mode == "process_requests"sv) {

        // process requests here

    } else {
        PrintUsage();
        return 1;
    }
}