#ifndef DIAGNOSTICS_HPP
#define DIAGNOSTICS_HPP

#include <string>
#include <vector>

#include "sensor_data.hpp"

enum class DiagnosticSeverity {
    WARNING,
    ERROR
};

struct DiagnosticEvent {
    DiagnosticSeverity severity;
    std::string code;
    std::string message;
};

std::vector<DiagnosticEvent> evaluateSensorData(
    const SensorData& data
);

#endif