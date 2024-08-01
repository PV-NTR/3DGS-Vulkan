#pragma once

#include <cstdint>

namespace X {
    
extern uint16_t FloatToHalf(float value);
extern uint32_t PackHalf2x16(float x, float y);

} // namespace X