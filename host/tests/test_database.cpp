#include <gtest/gtest.h>
#include <sqlite3.h>
#include <cstdio>

#include "database.hpp"

class DatabaseTest : public ::testing::Test {
protected:
    sqlite3* db = nullptr;
    const char* filename = "test_pulsecore.db";

    void SetUp() override {
        std::remove(filename);

        ASSERT_TRUE(initializeDatabase(&db, filename));
        ASSERT_NE(db, nullptr);
    }

    void TearDown() override {
        closeDatabase(db);
        db = nullptr;

        std::remove(filename);
    }
};

// Test 1: Database and table creation
TEST_F(DatabaseTest, CreatesDatabaseAndTable)
{
    sqlite3_stmt* statement = nullptr;

    const char* sql =
        "SELECT name FROM sqlite_master "
        "WHERE type='table' AND name='sensor_data';";

    ASSERT_EQ(
        sqlite3_prepare_v2(db, sql, -1, &statement, nullptr),
        SQLITE_OK
    );

    EXPECT_EQ(sqlite3_step(statement), SQLITE_ROW);

    sqlite3_finalize(statement);
}

// Test 2: Successful sensor-data insertion
TEST_F(DatabaseTest, InsertsSensorData)
{
    SensorData data{24.5f, 46.2f, 100.45f};

    EXPECT_TRUE(insertSensorData(db, data));

    sqlite3_stmt* statement = nullptr;

    ASSERT_EQ(
        sqlite3_prepare_v2(
            db,
            "SELECT temperature, humidity, pressure "
            "FROM sensor_data;",
            -1,
            &statement,
            nullptr
        ),
        SQLITE_OK
    );

    ASSERT_EQ(sqlite3_step(statement), SQLITE_ROW);

    EXPECT_FLOAT_EQ(
        static_cast<float>(sqlite3_column_double(statement, 0)),
        24.5f
    );

    EXPECT_FLOAT_EQ(
        static_cast<float>(sqlite3_column_double(statement, 1)),
        46.2f
    );

    EXPECT_FLOAT_EQ(
        static_cast<float>(sqlite3_column_double(statement, 2)),
        100.45f
    );

    sqlite3_finalize(statement);
}

// Test 3: Multiple sensor-data insertions
TEST_F(DatabaseTest, InsertsMultipleRecords)
{
    SensorData first{24.5f, 46.2f, 100.45f};
    SensorData second{25.0f, 47.0f, 101.0f};

    ASSERT_TRUE(insertSensorData(db, first));
    ASSERT_TRUE(insertSensorData(db, second));

    EXPECT_EQ(sqlite3_total_changes(db), 2);
}

// Test 4: Data survives database reopening
TEST_F(DatabaseTest, PreservesDataAfterReopening)
{
    SensorData data{24.5f, 46.2f, 100.45f};

    ASSERT_TRUE(insertSensorData(db, data));

    closeDatabase(db);
    db = nullptr;

    ASSERT_TRUE(initializeDatabase(&db, filename));

    sqlite3_stmt* statement = nullptr;

    ASSERT_EQ(
        sqlite3_prepare_v2(
            db,
            "SELECT COUNT(*) FROM sensor_data;",
            -1,
            &statement,
            nullptr
        ),
        SQLITE_OK
    );

    ASSERT_EQ(sqlite3_step(statement), SQLITE_ROW);

    EXPECT_EQ(sqlite3_column_int(statement, 0), 1);

    sqlite3_finalize(statement);
}

// Test 5: Diagnostic events table creation
TEST_F(DatabaseTest, CreatesDiagnosticEventsTable)
{
    sqlite3_stmt* statement = nullptr;

    const char* sql =
        "SELECT name FROM sqlite_master "
        "WHERE type='table' AND name='diagnostic_events';";

    ASSERT_EQ(
        sqlite3_prepare_v2(db, sql, -1, &statement, nullptr),
        SQLITE_OK
    );

    EXPECT_EQ(sqlite3_step(statement), SQLITE_ROW);

    sqlite3_finalize(statement);
}

// Test 6: Successful diagnostic event insertion
TEST_F(DatabaseTest, InsertsDiagnosticEvent)
{
    DiagnosticEvent event{
        DiagnosticSeverity::WARNING,
        "TEMP_OUT_OF_RANGE",
        "Temperature is outside the configured limits"
    };

    ASSERT_TRUE(insertDiagnosticEvent(db, event));

    sqlite3_stmt* statement = nullptr;

    ASSERT_EQ(
        sqlite3_prepare_v2(
            db,
            "SELECT severity, code, message FROM diagnostic_events;",
            -1,
            &statement,
            nullptr
        ),
        SQLITE_OK
    );

    ASSERT_EQ(sqlite3_step(statement), SQLITE_ROW);

    EXPECT_STREQ(
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 0)),
        "WARNING"
    );

    EXPECT_STREQ(
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)),
        "TEMP_OUT_OF_RANGE"
    );

    EXPECT_STREQ(
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)),
        "Temperature is outside the configured limits"
    );

    sqlite3_finalize(statement);
}

// Test 7: Multiple diagnostic event insertions
TEST_F(DatabaseTest, InsertsMultipleDiagnosticEvents)
{
    DiagnosticEvent first{
        DiagnosticSeverity::WARNING,
        "TEMP_OUT_OF_RANGE",
        "Temperature is outside the configured limits"
    };

    DiagnosticEvent second{
        DiagnosticSeverity::ERROR,
        "PRESS_OUT_OF_RANGE",
        "Pressure is outside the configured limits"
    };

    ASSERT_TRUE(insertDiagnosticEvent(db, first));
    ASSERT_TRUE(insertDiagnosticEvent(db, second));

    sqlite3_stmt* statement = nullptr;

    ASSERT_EQ(
        sqlite3_prepare_v2(
            db,
            "SELECT COUNT(*) FROM diagnostic_events;",
            -1,
            &statement,
            nullptr
        ),
        SQLITE_OK
    );

    ASSERT_EQ(sqlite3_step(statement), SQLITE_ROW);

    EXPECT_EQ(sqlite3_column_int(statement, 0), 2);

    sqlite3_finalize(statement);
}