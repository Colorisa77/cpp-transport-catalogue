#pragma once

#include "transport_catalogue.h"
#include "json_reader.h"

#include <filesystem>

namespace serialization {
    void SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& transport_catalogue, json_reader::JsonReader& json_reader);
    void DeserializeTransportCatalogue(transport_catalogue::TransportCatalogue& transport_catalogue);

} // namespace serialization