cmake_minimum_required(VERSION 3.12.1)
project(18_transport_catalogue_proto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -O2")

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto graph.proto)

set(UTILITY geo.h geo.cpp ranges.h)
set(TRANSPORT_CATALOGUE transport_catalogue.h transport_catalogue.cpp domain.h domain.cpp serialization.cpp serialization.h transport_catalogue.proto svg.proto map_renderer.proto graph.proto)
set(ROUTER graph.h transport_router.h transport_router.cpp)
set(JSON json.cpp json.h json_builder.h json_builder.cpp json_reader.h json_reader.cpp)
set(SVG svg.h svg.cpp)
set(MAP_RENDER map_renderer.h map_renderer.cpp)
set(REQUEST_HANDLER request_handler.h request_handler.cpp)
add_executable(18_transport_catalogue_proto main.cpp ${PROTO_SRCS} ${PROTO_HDRS} ${UTILITY} ${TRANSPORT_CATALOGUE} ${ROUTER} ${JSON} ${SVG} ${MAP_RENDER} ${REQUEST_HANDLER})

target_include_directories(18_transport_catalogue_proto PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(18_transport_catalogue_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(18_transport_catalogue_proto "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
