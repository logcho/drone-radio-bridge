#include "packet_encoder.hpp"
#include "crc16.hpp"
#include <cstring> // for memcpy


// temp CRC placeholder
static uint16_t fake_crc16(const uint8_t* data, size_t len){
    return 0xABCD;  // placeholder until real CRC added
}

std::vector<uint8_t> Encoder::encodeCmdVel(uint8_t seq, float lin_x, float ang_z){
    uint8_t msg_id = 0x02; // CMD_VEL

    // build payload 8 bytes 2 floats
    uint8_t payload[8];
    std::memcpy(&payload[0], &lin_x, 4);
    std::memcpy(&payload[4], &ang_z, 4);

    // build body [msg_id][seq][payload]
    std::vector<uint8_t> body;
    body.push_back(msg_id);
    body.push_back(seq);
    body.insert(body.end(), payload, payload + 8);

    // CRC of body
    uint16_t crc = crc16_ccitt_false(body.data(), body.size());

    // LEN field
    uint8_t len = body.size() + 2;  // body + crc

    // build final packet
    std::vector<uint8_t> pkt;

    pkt.push_back(0xAA);
    pkt.push_back(0x55);
    pkt.push_back(len);

    pkt.insert(pkt.end(), body.begin(), body.end());

    // append CRC
    pkt.push_back(crc & 0xFF);
    pkt.push_back((crc >> 8) & 0xFF);

    return pkt;
}