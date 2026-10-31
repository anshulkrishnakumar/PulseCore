#include "telemetry_parser.hpp"
#include <cstdio>
#include <cmath>

bool parseTelemetryLine(const std::string& line, SensorData& data) {
    float temperature;
    float humidity;
    float pressure;

    int result = std::sscanf(
        line.c_str(),
        "TEMP:%f,HUM:%f,PRESS:%f",
        &temperature,
        &humidity,
        &pressure
    );

    if (result != 3) {
        return false;
    }

    if (!std::isfinite(temperature) ||
    !std::isfinite(humidity) ||
    !std::isfinite(pressure)) {
    return false;
    }

    if (temperature < -40.0f || temperature > 85.0f ||
        humidity < 0.0f || humidity > 100.0f ||
        pressure <= 0.0f) {
        return false;
    }

    data.temperature = temperature;
    data.humidity = humidity;
    data.pressure = pressure;

    return true;
}