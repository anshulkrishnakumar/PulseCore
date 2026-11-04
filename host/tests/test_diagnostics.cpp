#include <gtest/gtest.h>

#include "diagnostics.hpp"

TEST(DiagnosticsTest, AcceptsNormalSensorReadings)
{
    SensorData data{24.0f, 45.0f, 100.0f};

    auto events = evaluateSensorData(data);

    EXPECT_TRUE(events.empty());
}

TEST(DiagnosticsTest, DetectsHighTemperature)
{
    SensorData data{55.0f, 45.0f, 100.0f};

    auto events = evaluateSensorData(data);

    ASSERT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].code, "TEMP_OUT_OF_RANGE");
}

TEST(DiagnosticsTest, DetectsLowHumidity)
{
    SensorData data{24.0f, 10.0f, 100.0f};

    auto events = evaluateSensorData(data);

    ASSERT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].code, "HUM_OUT_OF_RANGE");
}

TEST(DiagnosticsTest, DetectsMultipleAbnormalReadings)
{
    SensorData data{55.0f, 10.0f, 100.0f};

    auto events = evaluateSensorData(data);

    ASSERT_EQ(events.size(), 2);
}