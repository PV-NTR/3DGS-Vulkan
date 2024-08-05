#pragma once

#include <cstdint>
#include <type_traits>
#include <string>

extern uint32_t NewID();

template <typename E>
constexpr typename std::underlying_type_t<E> ECast(E e)
{
    return static_cast<std::underlying_type_t<E>>(e);
}

inline const std::string GetAssetPath()
{
#ifdef HOST_ANDROID
    return "models";
#elif defined ASSET_DIR
    return ASSET_DIR;
#else
    return "";
#endif
}

inline const std::string GetShaderPath()
{
#ifdef HOST_ANDROID
    return "shaders";
#elif defined SHADER_DIR
    return SHADER_DIR;
#else
    return "";
#endif
}