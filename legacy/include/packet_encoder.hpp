#pragma once
#include <vector>
#include <cstdint>

namespace Encoder {

    std::vector<uint8_t> encodeCmdVel(uint8_t seq, float lin_x, float ang_z);
    
};