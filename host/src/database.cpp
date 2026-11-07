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

    const char* diagnosticTableSQL = R"(
    CREATE TABLE IF NOT EXISTS diagnostic_events (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
        severity TEXT NOT NULL,
        code TEXT NOT NULL,
        message TEXT NOT NULL);
        )";

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

    // Create diagnostic events table
    error_message = nullptr;

    result = sqlite3_exec(
        *db,
        diagnosticTableSQL,
        nullptr,
        nullptr,
        &error_message
    );

    if (result != SQLITE_OK) {
        std::cerr << "Diagnostic table creation failed: "
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

bool insertDiagnosticEvent(
    sqlite3* db,
    const DiagnosticEvent& event)
{
    const char* sql =
        "INSERT INTO diagnostic_events "
        "(severity, code, message) "
        "VALUES (?, ?, ?);";

    sqlite3_stmt* statement = nullptr;

    int result = sqlite3_prepare_v2(
        db, sql, -1, &statement, nullptr
    );

    if (result != SQLITE_OK) {
        std::cerr << "Failed to prepare diagnostic statement: "
                  << sqlite3_errmsg(db) << "\n";
        return false;
    }

    const char* severity =
        event.severity == DiagnosticSeverity::WARNING
            ? "WARNING"
            : "ERROR";

    sqlite3_bind_text(statement, 1, severity, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, event.code.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, event.message.c_str(), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(statement);

    if (result != SQLITE_DONE) {
        std::cerr << "Failed to insert diagnostic event: "
                  << sqlite3_errmsg(db) << "\n";

        sqlite3_finalize(statement);
        return false;
    }

    sqlite3_finalize(statement);
    return true;
}

void closeDatabase(sqlite3* db)
{
    if (db != nullptr) {
        sqlite3_close(db);
    }
}