#ifndef TELEMETRY_PARSER_HPP
#define TELEMETRY_PARSER_HPP

#include <string>
#include "sensor_data.hpp"

bool parseTelemetryLine(const std::string& line, SensorData& data);

#endif