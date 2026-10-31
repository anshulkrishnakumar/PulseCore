#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int main() {
    const char* port = "/dev/ttyACM0";

    int serial_fd = open(port, O_RDONLY | O_NOCTTY); // read only + no controlling terminal

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
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        std::cerr << "Error: Could not configure serial port\n";
        close(serial_fd);
        return 1;
    }

    std::cout << "PulseCore: Connected to " << port << "\n";

    char buffer[256];

    while (true) {
        ssize_t bytes_read = read(serial_fd, buffer, sizeof(buffer));

        if (bytes_read > 0) {
            std::cout.write(buffer, bytes_read);
            std::cout.flush();
        } else if (bytes_read < 0) {
            std::cerr << "Error reading serial data\n";
            break;
        }
    }

    close(serial_fd);
    return 0;
}