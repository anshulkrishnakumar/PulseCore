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