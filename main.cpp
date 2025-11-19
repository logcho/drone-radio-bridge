#include "serial_port.hpp"
#include <iostream>
#include <unistd.h>  // unsleep


int main() {
    SerialPort sp("/dev/tty.Bluetooth-Incoming-Port", 57600); // macos serial device

    if (!sp.openPort()) {
        std::cout << "Failed to open port.\n";
        return 1;
    }

    std::cout << "Serial port opened successfully.\n";

    uint8_t buf[64];

    while (true) {
        int n = sp.readBytes(buf, sizeof(buf));
        if (n > 0) {
            std::cout << "Received " << n << " bytes: ";
            for (int i = 0; i < n; ++i)
                printf("%02X ", buf[i]);
            std::cout << std::endl;
        }

        // test write
        const char* msg = "Hello\n";
        sp.writeBytes((const uint8_t*)msg, strlen(msg));

        usleep(100000); // 100ms
    }

    return 0;
}