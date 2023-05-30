#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

void QueryProcessing(transport_catalogue::TransportCatalogue& transport_catalogue, std::istream& input, std::ostream& output) {
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
}