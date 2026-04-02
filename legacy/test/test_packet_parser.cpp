#include "packet_parser.hpp"
#include "packet_encoder.hpp"
#include <iostream>

int main() {
    PacketParser parser;

    // Encoder packet
    uint8_t seq = 0x22;
    float lin_x = 1.23f;
    float ang_z = -4.56f;

    std::vector<uint8_t> pkt = Encoder::encodeCmdVel(seq, lin_x, ang_z);

    // Print the encoded packet:
    std::cout << "Encoded: ";
    for (auto b : pkt) printf("%02X ", b);
    std::cout << "\n";

    // Parse the encoded packet
    auto decoded = parser.feed(pkt);

    // Verify
    if (decoded.size() == 1) {
        auto &p = decoded[0];

        std::cout << "Decoded OK!\n";
        std::cout << "msg_id = " << int(p.msg_id) << "\n";
        std::cout << "seq    = " << int(p.seq) << "\n";
        std::cout << "payload bytes = " << p.payload.size() << "\n";

        // interpret floats back out
        float out_lin_x, out_ang_z;
        memcpy(&out_lin_x, &p.payload[0], 4);
        memcpy(&out_ang_z, &p.payload[4], 4);

        std::cout << "Decoded lin_x = " << out_lin_x << "\n";
        std::cout << "Decoded ang_z = " << out_ang_z << "\n";
    }
    else {
        std::cout << "Parser FAILED (0 packets decoded)\n";
    }
}
