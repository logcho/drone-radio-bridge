#include "packet_encoder.hpp"
#include <iostream>

int main() {
    auto pkt = Encoder::encodeCmdVel(0x10, 1.0f, -0.5f);

    std::cout << "Encoded packet (" << pkt.size() << " bytes): ";
    for (uint8_t b : pkt) {
        printf("%02X ", b);
    }
    std::cout << std::endl;

    return 0;
}
