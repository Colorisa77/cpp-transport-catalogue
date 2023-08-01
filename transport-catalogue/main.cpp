#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

#include <fstream>
#include <iostream>
#include <string_view>

void QueryProcessing(transport_catalogue::TransportCatalogue& transport_catalogue,
                     renderer::RenderSettings& render_settings,
                     graph::RouteSettings& route_settings,
                     json_reader::JsonReader& json_reader,
                     std::ostream& output) {

    router::TransportRouter router;
    renderer::MapRenderer map_render;
    request_handler::RequestHandler request_handler(transport_catalogue, router, map_render);
    json_reader::SequentialRequestProcessing(
        transport_catalogue,
        render_settings,
        route_settings,
        router, 
        map_render, 
        json_reader,
        output, 
        request_handler
    );
}

void MakeBase(transport_catalogue::TransportCatalogue& transport_catalogue,
              renderer::RenderSettings& render_settings,
              graph::RouteSettings& route_settigns,
              json_reader::JsonReader& json_reader) {

    transport_catalogue_proto::Catalogue catalogue;

    json_reader::FillingTransportCatalogue(transport_catalogue, json_reader);
    serialization::SerializeTransportCatalogue(catalogue, transport_catalogue);
    render_settings = json_reader::SetRenderSettings(json_reader.GetRenderSettings());
    route_settigns = json_reader::SetRoutingSettings(json_reader.GetRouteSettings());
    serialization::SerializeRenderSettings(catalogue, render_settings);
    serialization::SerializeRouteSettings(catalogue, route_settigns);
    serialization::SerializeCatalogue(catalogue, json_reader);

}

void ProcessRequests(transport_catalogue::TransportCatalogue& transport_catalogue,
                     renderer::RenderSettings& render_settings,
                     graph::RouteSettings& route_settigns,
                     json_reader::JsonReader& json_reader,
                     std::ostream& output) {
    transport_catalogue_proto::Catalogue catalogue_proto;

    serialization::DeserializeCatalogue(catalogue_proto, json_reader);
    serialization::DeserializeTransportCatalogue(transport_catalogue, catalogue_proto);
    serialization::DeserializeRenderSettings(render_settings, catalogue_proto);
    serialization::DeserializeRouteSettings(route_settigns, catalogue_proto);
    QueryProcessing(transport_catalogue, render_settings, route_settigns, json_reader, output);
}

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    transport_catalogue::TransportCatalogue transport_catalogue;
    renderer::RenderSettings render_settings;
    graph::RouteSettings route_settings;
    json_reader::JsonReader json_reader(std::cin);

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        // make base here
        MakeBase(transport_catalogue, render_settings, route_settings, json_reader);

    } else if (mode == "process_requests"sv) {
        // process requests here
        std::ofstream output("output.json");
        ProcessRequests(transport_catalogue, render_settings, route_settings, json_reader, output);

    } else {
        PrintUsage();
        return 1;
    }
}