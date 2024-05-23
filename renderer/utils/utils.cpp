#include "utils/utils.hpp"

#include <atomic>
#include <cassert>

std::atomic<uint32_t> g_currentID = { 1 };

uint32_t NewID()
{
    assert(g_currentID != UINT32_MAX);
    return g_currentID.fetch_add(1, std::memory_order_relaxed);
}