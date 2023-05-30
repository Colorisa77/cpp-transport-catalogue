cmake_minimum_required(VERSION 3.12.1)
project(18_transport_catalogue_proto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -std=gnu++11 -std=c++17 -g -O2")

set(UTILITY geo.h geo.cpp ranges.h)
set(TRANSPORT_CATALOGUE transport_catalogue.h transport_catalogue.cpp domain.h domain.cpp)
set(ROUTER graph.h transport_router.h transport_router.cpp)
set(JSON json.cpp json.h json_builder.h json_builder.cpp json_reader.h json_reader.cpp)
set(SVG svg.h svg.cpp)
set(MAP_RENDER map_renderer.h map_renderer.cpp)
set(REQUEST_HANDLER request_handler.h request_handler.cpp)
add_executable(18_transport_catalogue_proto transport_catalogue main.cpp ${UTILITY} ${TRANSPORT_CATALOGUE} ${ROUTER} ${JSON} ${SVG} ${MAP_RENDER} ${REQUEST_HANDLER})
