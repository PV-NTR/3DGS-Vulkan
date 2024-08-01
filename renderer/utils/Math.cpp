#include "Math.hpp"

namespace X {

uint16_t FloatToHalf(float value)
{
    uint32_t f = *reinterpret_cast<uint32_t*>(&value);

    uint32_t sign = (f >> 31) & 0x0001;
    uint32_t exp = (f >> 23) & 0x00ff;
    uint32_t frac = f & 0x007fffff;

    uint32_t newExp;
    if (exp == 0) {
        newExp = 0;
    } else if (exp < 113) {
        newExp = 0;
        frac |= 0x00800000;
        frac >>= (113 - exp);
        if (frac & 0x01000000) {
            newExp = 1;
            frac = 0;
        }
    } else if (exp < 142) {
        newExp = exp - 112;
    } else {
        newExp = 31;
        frac = 0;
    }

    return (sign << 15) | (newExp << 10) | (frac >> 13);
}

uint32_t PackHalf2x16(float x, float y)
{
    uint16_t hx = FloatToHalf(x);
    uint16_t hy = FloatToHalf(y);
    return (uint32_t)hx | ((uint32_t)hy << 16);
}

} // namespace X