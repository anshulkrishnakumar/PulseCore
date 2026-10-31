#include "database.hpp"

#include <sqlite3.h>
#include <iostream>

bool initializeDatabase(sqlite3** db, const char* filename)
{
    int result = sqlite3_open(filename, db);

    if (result != SQLITE_OK) {
        std::cerr << "Database error: "
                  << sqlite3_errmsg(*db) << "\n";

        sqlite3_close(*db);
        *db = nullptr;
        return false;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS sensor_data ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "temperature REAL NOT NULL,"
        "humidity REAL NOT NULL,"
        "pressure REAL NOT NULL"
        ");";

    char* error_message = nullptr;

    result = sqlite3_exec(*db, sql, nullptr, nullptr, &error_message);

    if (result != SQLITE_OK) {
        std::cerr << "Table creation failed: "
                  << error_message << "\n";

        sqlite3_free(error_message);
        sqlite3_close(*db);
        *db = nullptr;
        return false;
    }

    return true;
}

bool insertSensorData(sqlite3* db, const SensorData& data)
{
    const char* sql =
        "INSERT INTO sensor_data (temperature, humidity, pressure) "
        "VALUES (?, ?, ?);";

    sqlite3_stmt* statement = nullptr;

    int result = sqlite3_prepare_v2(
        db, sql, -1, &statement, nullptr
    );

    if (result != SQLITE_OK) {
        std::cerr << "SQL preparation failed: "
                  << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_double(statement, 1, data.temperature);
    sqlite3_bind_double(statement, 2, data.humidity);
    sqlite3_bind_double(statement, 3, data.pressure);

    result = sqlite3_step(statement);

    if (result != SQLITE_DONE) {
        std::cerr << "Database insertion failed: "
                  << sqlite3_errmsg(db) << "\n";
    }

    sqlite3_finalize(statement);

    return result == SQLITE_DONE;
}

void closeDatabase(sqlite3* db)
{
    if (db != nullptr) {
        sqlite3_close(db);
    }
}