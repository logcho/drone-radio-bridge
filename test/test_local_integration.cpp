#include "serial_port.hpp"
#include "packet_parser.hpp"
#include "packet_encoder.hpp"

#include <iostream>
#include <vector>
#include <unistd.h>
#include <cstring>

int main() {
    // Use any serial port that opens on macOS
    SerialPort serial("/dev/tty.Bluetooth-Incoming-Port", 57600);

    if (!serial.openPort()) {
        std::cerr << "Failed to open serial port\n";
        return 1;
    }

    std::cout << "Serial opened\n";

    PacketParser parser;
    uint8_t seq = 0;
    uint8_t rx_buf[256];

    while (true) {
        // ---- READ ----
        int n = serial.readBytes(rx_buf, sizeof(rx_buf));
        if (n > 0) {
            std::vector<uint8_t> data(rx_buf, rx_buf + n);
            auto packets = parser.feed(data);

            for (auto &p : packets) {
                std::cout << "[RX] msg_id=" << int(p.msg_id)
                          << " seq=" << int(p.seq)
                          << " payload=" << p.payload.size()
                          << "\n";
            }
        }

        // ---- SEND ----
        auto pkt = Encoder::encodeCmdVel(seq++, 1.0f, -0.5f);
        serial.writeBytes(pkt.data(), pkt.size());

        std::cout << "[TX] Sent CMD_VEL seq=" << int(seq - 1) << "\n";

        usleep(200000); // 200 ms
    }
}
