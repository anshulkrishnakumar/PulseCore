#include "diagnostics.hpp"

std::vector<DiagnosticEvent> evaluateSensorData(
    const SensorData& data)
{
    std::vector<DiagnosticEvent> events;

    if (data.temperature < 0.0f || data.temperature > 50.0f) {
        events.push_back({
            DiagnosticSeverity::WARNING,
            "TEMP_OUT_OF_RANGE",
            "Temperature is outside the configured limits"
        });
    }

    if (data.humidity < 20.0f || data.humidity > 80.0f) {
        events.push_back({
            DiagnosticSeverity::WARNING,
            "HUM_OUT_OF_RANGE",
            "Humidity is outside the configured limits"
        });
    }

    if (data.pressure < 80.0f || data.pressure > 110.0f) {
        events.push_back({
            DiagnosticSeverity::WARNING,
            "PRESS_OUT_OF_RANGE",
            "Pressure is outside the configured limits"
        });
    }

    return events;
}