#include "serial_port.hpp"
#include "packet_parser.hpp"
#include "packet_encoder.hpp"

#include <iostream>
#include <vector>
#include <unistd.h>
#include <cstring>

int main() {
    // CHANGE THIS to the correct port on your system
    SerialPort serial("/dev/tty.Bluetooth-Incoming-Port", 57600);

    if (!serial.openPort()) {
        std::cerr << "Failed to open serial port\n";
        return 1;
    }

    std::cout << "Serial port opened\n";

    PacketParser parser;
    uint8_t seq = 0;

    uint8_t rx_buf[256];

    while (true) {
        // ----------------------------
        // READ FROM SERIAL
        // ----------------------------
        int n = serial.readBytes(rx_buf, sizeof(rx_buf));
        if (n > 0) {
            std::vector<uint8_t> data(rx_buf, rx_buf + n);

            auto packets = parser.feed(data);
            for (auto &p : packets) {
                std::cout << "[RX] msg_id=" << int(p.msg_id)
                          << " seq=" << int(p.seq)
                          << " payload=" << p.payload.size()
                          << " bytes\n";
            }
        }

        // ----------------------------
        // SEND A TEST PACKET
        // ----------------------------
        float lin_x = 1.0f;
        float ang_z = -0.5f;

        auto pkt = Encoder::encodeCmdVel(seq++, lin_x, ang_z);
        serial.writeBytes(pkt.data(), pkt.size());

        std::cout << "[TX] Sent CMD_VEL seq=" << int(seq - 1) << "\n";

        usleep(200000); // 200 ms
    }

    return 0;
}
