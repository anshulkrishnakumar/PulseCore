#include <gtest/gtest.h>

#include "telemetry_parser.hpp"

// Test 1: Valid telemetry record
TEST(TelemetryParserTest, ParsesValidRecord)
{
    std::string line = "TEMP:24.5,HUM:46.2,PRESS:100.45";
    SensorData data{};

    bool result = parseTelemetryLine(line, data);

    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(data.temperature, 24.5f);
    EXPECT_FLOAT_EQ(data.humidity, 46.2f);
    EXPECT_FLOAT_EQ(data.pressure, 100.45f);
}

// Test 2: Malformed telemetry record
TEST(TelemetryParserTest, RejectsMalformedRecord)
{
    std::string line = "TEMP:24.5,HUM:46.2";
    SensorData data{};

    EXPECT_FALSE(parseTelemetryLine(line, data));
}

// Test 3: Temperature outside valid range
TEST(TelemetryParserTest, RejectsInvalidTemperature)
{
    std::string line = "TEMP:999,HUM:46.2,PRESS:100.45";
    SensorData data{};

    EXPECT_FALSE(parseTelemetryLine(line, data));
}

// Test 4: Humidity outside valid range
TEST(TelemetryParserTest, RejectsInvalidHumidity)
{
    std::string line = "TEMP:24.5,HUM:150,PRESS:100.45";
    SensorData data{};

    EXPECT_FALSE(parseTelemetryLine(line, data));
}

// Test 5: Invalid pressure
TEST(TelemetryParserTest, RejectsInvalidPressure)
{
    std::string line = "TEMP:24.5,HUM:46.2,PRESS:0";
    SensorData data{};

    EXPECT_FALSE(parseTelemetryLine(line, data));
}

// Test 6: Rejects non-finite values
TEST(TelemetryParserTest, RejectsNonFiniteValues)
{
    std::string line = "TEMP:nan,HUM:46.2,PRESS:100.45";
    SensorData data{};

    EXPECT_FALSE(parseTelemetryLine(line, data));
}

// Test 7: Accept minimum temperature
TEST(TelemetryParserTest, AcceptsMinimumTemperature)
{
    std::string line = "TEMP:-40,HUM:50,PRESS:100";
    SensorData data{};

    EXPECT_TRUE(parseTelemetryLine(line, data));
    EXPECT_FLOAT_EQ(data.temperature, -40.0f);
}

// Test 8: Accept maximum temperature
TEST(TelemetryParserTest, AcceptsMaximumTemperature)
{
    std::string line = "TEMP:85,HUM:50,PRESS:100";
    SensorData data{};

    EXPECT_TRUE(parseTelemetryLine(line, data));
    EXPECT_FLOAT_EQ(data.temperature, 85.0f);
}

// Test 9: Accept minimum humidity
TEST(TelemetryParserTest, AcceptsMinimumHumidity)
{
    std::string line = "TEMP:25,HUM:0,PRESS:100";
    SensorData data{};

    EXPECT_TRUE(parseTelemetryLine(line, data));
    EXPECT_FLOAT_EQ(data.humidity, 0.0f);
}

// Test 10: Accept maximum humidity
TEST(TelemetryParserTest, AcceptsMaximumHumidity)
{
    std::string line = "TEMP:25,HUM:100,PRESS:100";
    SensorData data{};

    EXPECT_TRUE(parseTelemetryLine(line, data));
    EXPECT_FLOAT_EQ(data.humidity, 100.0f);
}

// Test 11: Reject trailing characters in temperature
TEST(TelemetryParserTest, RejectsTrailingCharactersInTemperature)
{
    std::string line = "TEMP:24.5abc,HUM:46.2,PRESS:100.45";
    SensorData data{};

    EXPECT_FALSE(parseTelemetryLine(line, data));
}

// Test 12: Reject trailing characters in pressure
TEST(TelemetryParserTest, RejectsTrailingCharactersInPressure)
{
    std::string line = "TEMP:24.5,HUM:46.2,PRESS:100.45xyz";
    SensorData data{};

    EXPECT_FALSE(parseTelemetryLine(line, data));
}

// Test 13: Reject missing fields
TEST(TelemetryParserTest, RejectsMissingPressure)
{
    std::string line = "TEMP:24.5,HUM:46.2";
    SensorData data{};

    EXPECT_FALSE(parseTelemetryLine(line, data));
}

// Test 14: Reject extra fields
TEST(TelemetryParserTest, RejectsExtraFields)
{
    std::string line = "TEMP:24.5,HUM:46.2,PRESS:100.45,ALT:20";
    SensorData data{};

    EXPECT_FALSE(parseTelemetryLine(line, data));
}