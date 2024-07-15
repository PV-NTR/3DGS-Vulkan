#pragma once

#include <cstdint>

namespace Android {

/* @brief Touch control thresholds from Android NDK samples */
constexpr int32_t DOUBLE_TAP_TIMEOUT = 300 * 1000000;
constexpr int32_t TAP_TIMEOUT = 180 * 1000000;
constexpr int32_t DOUBLE_TAP_SLOP = 100;
constexpr int32_t TAP_SLOP = 8;

} // namespace Android