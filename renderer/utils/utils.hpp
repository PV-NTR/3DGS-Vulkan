#pragma once

#include <cstdint>
#include <type_traits>

extern uint32_t NewID();

template <typename E>
constexpr typename std::underlying_type_t<E> ECast(E e)
{
    return static_cast<std::underlying_type_t<E>>(e);
}