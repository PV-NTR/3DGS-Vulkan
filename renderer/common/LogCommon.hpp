#pragma once

#include <regex>

#include "plog/Log.h"
#include "plog/Initializers/RollingFileInitializer.h"
#include "plog/Formatters/TxtFormatter.h"
#ifdef HOST_ANDROID
#include "plog/Appenders/AndroidAppender.h"
#else
#include "plog/Appenders/ColorConsoleAppender.h"
#endif

namespace X {

inline ::std::string FormatWithLogOutput(const char* format)
{
    return "X: " + std::regex_replace(format, std::regex(R"(\{public\})"), "") + "\r\n";
}

#define INIT_LOGGER plog::init
#define XLOGE(fmt, ...) PLOGE.printf(::X::FormatWithLogOutput(fmt).c_str(), ##__VA_ARGS__)
#define XLOGI(fmt, ...) PLOGI.printf(::X::FormatWithLogOutput(fmt).c_str(), ##__VA_ARGS__)
#define XLOGW(fmt, ...) PLOGW.printf(::X::FormatWithLogOutput(fmt).c_str(), ##__VA_ARGS__)
#define XLOGD(fmt, ...) PLOGD.printf(::X::FormatWithLogOutput(fmt).c_str(), ##__VA_ARGS__)

} // namespace X