#include "packet_parser.hpp"
#include "crc16.hpp"
#include <iostream>

PacketParser::PacketParser() {}

std::vector<DecodedPacket> PacketParser::feed(const std::vector<uint8_t>& data){
    std::vector<DecodedPacket> out;
    
    // put incoming bytes into buffer
    buffer_.insert(buffer_.end(), data.begin(), data.end());

    // try extracting bytes
    while(buffer_.size() >= 4){

        if(buffer_[0] != 0xAA || buffer_[1] != 0x55){
            // std::cerr << "Parser: Bad sync byte: 0x" << std::hex << (int)buffer_[0] << std::dec << "\n";
            buffer_.erase(buffer_.begin());
            continue;
        }

        // read in LEN field (payload_length + CRC + msg_id + seq)
        uint8_t len = buffer_[2];
        size_t total_len = 3 + len; // header (2), len(1), body + crc

        // not enought bytes yet?
        if(buffer_.size() < total_len) break;

        // extract packet 
        std::vector<uint8_t> packet(buffer_.begin(), buffer_.begin() + total_len);
        buffer_.erase(buffer_.begin(), buffer_.begin() + total_len);

        // Validate CRC
        uint16_t crc_expected = packet[total_len - 2] | (packet[total_len - 1] << 8);

        uint16_t crc_calc = crc16_ccitt_false(packet.data() + 3, total_len - 5);
        // ^ packet body = MSG_ID, SEQ, PAYLOAD…

        if (crc_calc != crc_expected) {
            std::cerr << "Parser: CRC mismatch! calc=0x" << std::hex << crc_calc 
                      << " expected=0x" << crc_expected << std::dec << " (discarding packet)\n";
            continue;
        }

        // parse packet field 
        uint8_t msg_id = packet[3];
        uint8_t seq = packet[4];

        // payload = data between seq and crc
        std::vector<uint8_t> payload(packet.begin() + 5, packet.end() - 2);

        out.push_back({msg_id, seq, payload});
    }

    return out;
}
