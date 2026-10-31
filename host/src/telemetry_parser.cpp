#include "telemetry_parser.hpp"
#include <cstdio>
#include <cmath>

bool parseTelemetryLine(const std::string& line, SensorData& data) {
    float temperature;
    float humidity;
    float pressure;

    int consumed = 0;

    int result = std::sscanf(
        line.c_str(),
        "TEMP:%f,HUM:%f,PRESS:%f%n",
        &temperature,
        &humidity,
        &pressure,
        &consumed
    );

    if (result != 3 || consumed != static_cast<int>(line.size())) {
        return false;
    }

    if (!std::isfinite(temperature) ||
    !std::isfinite(humidity) ||
    !std::isfinite(pressure)) {
    return false;
    }

    if (temperature < -40.0f || temperature > 85.0f || // limits for BME280
        humidity < 0.0f || humidity > 100.0f ||
        pressure <= 0.0f) {
        return false;
    }

    data.temperature = temperature;
    data.humidity = humidity;
    data.pressure = pressure;

    return true;
}