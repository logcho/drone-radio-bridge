#include "packet_encoder.hpp"
#include "packet_parser.hpp"
#include <iostream>

int main() {
    PacketParser parser;
    
    auto pkt = Encoder::encodeCmdVel(0x22, 1.0f, -0.5f);

    auto decoded = parser.feed(pkt);

    if (decoded.size() == 1) {
        auto &p = decoded[0];
        std::cout << "Roundtrip OK! msg_id=" << int(p.msg_id)
                  << " seq=" << int(p.seq)
                  << " payload=" << p.payload.size() << std::endl;
    } else {
        std::cout << "Roundtrip FAILED." << std::endl;
    }
}
