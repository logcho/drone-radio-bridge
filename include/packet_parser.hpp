#pragma once
#include <vector>
#include <cstdint>

struct DecodedPacket {
    uint8_t msg_id;
    uint8_t seq;
    std::vector<uint8_t> payload;
};

class PacketParser {
    public:
        PacketParser();

        // feed raw bytes to parser
        std::vector<DecodedPacket> feed(const std::vector<uint8_t>& data);

    private:
        std::vector<uint8_t> buffer_; // internal byte buffer
};