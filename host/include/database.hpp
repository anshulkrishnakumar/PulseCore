#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "sensor_data.hpp"

struct sqlite3;

bool initializeDatabase(sqlite3** db, const char* filename); // creates database
bool insertSensorData(sqlite3* db, const SensorData& data); // saves a sensor measurement
void closeDatabase(sqlite3* db); // closes database connection

#endif
