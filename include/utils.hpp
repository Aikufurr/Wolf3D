#pragma once
#include <string>
#include <iomanip>

namespace utils {

template <typename T>
inline std::string valtostr(const T& val) {
    std::ostringstream sstream;
    sstream << val;
    return sstream.str();
}

template <typename T>
inline std::string valtostr(const T& val, int length, bool fixed = true) {
    std::ostringstream sstream;
    if (fixed) sstream << std::fixed;
    sstream << std::setprecision(length) << val;
    return sstream.str();
}

}  // namespace utils