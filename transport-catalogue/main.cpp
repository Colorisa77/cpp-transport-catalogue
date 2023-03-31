#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

void QueryProcessing(transport_catalogue::TransportCatalogue& transport_catalogue, std::istream& input, std::ostream& output) {
    renderer::MapRenderer map_render;
    request_handler::RequestHandler request_handler(transport_catalogue, map_render);
    json_reader::SequentialRequestProcessing(transport_catalogue, map_render, input, output, request_handler);
}

int main() {
    transport_catalogue::TransportCatalogue transport_catalogue;
    QueryProcessing(transport_catalogue, std::cin, std::cout);
}