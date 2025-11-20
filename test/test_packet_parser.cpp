#include "packet_parser.hpp"
#include <iostream>

int main() {
    PacketParser parser;

    // fake packet: AA 55 [LEN] [MSGID] [SEQ] [payload bytes] [CRC_L] [CRC_H]
    std::vector<uint8_t> bytes = {
        0xAA, 0x55, 0x07,   // header + total body len
        0x02, 0x10,         // msg_id, seq
        0x12, 0x34, 0x56,   // payload
        0xAB, 0xCD          // fake CRC
    };


    auto packets = parser.feed(bytes);

    for (auto& p : packets) {
        std::cout << "Got packet: msg_id=" << int(p.msg_id)
                  << " seq=" << int(p.seq)
                  << " payload bytes=" << p.payload.size()
                  << std::endl;
    }
}
