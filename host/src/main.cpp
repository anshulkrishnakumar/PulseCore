#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include "telemetry_parser.hpp"
#include "database.hpp"


int main()
{
    const char* port = "/dev/ttyACM0";

    int serial_fd = open(port, O_RDONLY | O_NOCTTY);

    if (serial_fd < 0) {
        std::cerr << "Error: Could not open serial port\n";
        return 1;
    }

    struct termios tty {};

    if (tcgetattr(serial_fd, &tty) != 0) {
        std::cerr << "Error: Could not get serial settings\n";
        close(serial_fd);
        return 1;
    }

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag |= CREAD | CLOCAL;

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | INLCR | IGNCR);
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        std::cerr << "Error: Could not configure serial port\n";
        close(serial_fd);
        return 1;
    }

    sqlite3* db = nullptr;

    if (!initializeDatabase(&db, "pulsecore.db")) {
        std::cerr << "Failed to initialize database\n";
        close(serial_fd);
        return 1;
    }

    std::cout << "PulseCore: Database initialized\n";

    std::cout << "PulseCore: Connected to " << port << "\n";

    std::string pending_line;
    char buffer[256];

    bool synchronized = false;

    while (true) {
        ssize_t bytes_read = read(serial_fd, buffer, sizeof(buffer));

        if (bytes_read < 0) {
            std::cerr << "Error reading serial data\n";
            break;
        }

        for (ssize_t i = 0; i < bytes_read; i++) {

            // ignore data until the first complete line arrives.
            if (!synchronized) {
                if (buffer[i] == '\n') {
                    synchronized = true;
                }

                continue;
            }

            if (buffer[i] == '\n') {
                if (!pending_line.empty() && pending_line.back() == '\r') {
                    pending_line.pop_back();
                }

                SensorData data;
                if (!pending_line.empty()) {
                    SensorData data;

                    if (parseTelemetryLine(pending_line, data)) {

                        if (!insertSensorData(db, data)) {
                            std::cerr << "Failed to save sensor data\n";
                        }

                        std::cout << "Temperature: " << data.temperature << " C\n";
                        std::cout << "Humidity: " << data.humidity << " %\n";
                        std::cout << "Pressure: " << data.pressure << " kPa\n";
                        std::cout << "-------------------------\n";
                    } else {
                        std::cerr << "Invalid telemetry record: "
                                << pending_line << "\n";
                    }
                }

                pending_line.clear();
            } else {
                pending_line += buffer[i];
            }
        }
    }
    closeDatabase(db);
    close(serial_fd);
    return 0;
}