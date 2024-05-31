#pragma once

#include <regex>

#include "plog/Log.h"
#include "plog/Initializers/RollingFileInitializer.h"

namespace X {

inline std::string FormatWithLogOutput(const char* format)
{
    return "X: " + std::regex_replace(format, std::regex(R"(\{public\})"), "") + "\r\n";
}

#define INIT_LOGGER plog::init
#define XLOGE(fmt, ...) PLOGE.printf(FormatWithLogOutput(fmt).c_str(), ##__VA_ARGS__)
#define XLOGI(fmt, ...) PLOGI.printf(FormatWithLogOutput(fmt).c_str(), ##__VA_ARGS__)
#define XLOGW(fmt, ...) PLOGW.printf(FormatWithLogOutput(fmt).c_str(), ##__VA_ARGS__)
#define XLOGD(fmt, ...) PLOGD.printf(FormatWithLogOutput(fmt).c_str(), ##__VA_ARGS__)

} // namespace X