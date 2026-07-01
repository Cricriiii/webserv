#include "tools/char_tools.hpp"

std::string char_tools::itostr(int nb) {
    std::stringstream ss;
    ss << nb;
    return ss.str();
}
#include <iostream>

void char_tools::trim_string(std::string& string, char trimmed) {
    size_t idx;

    for (idx = 0; idx != string.size(); ++idx) {
        if (string[idx] != trimmed)
            break;
    }
    string.erase(0, idx);

    for (idx = string.size() - 1; idx >= 0; --idx) {
        if (string[idx] != trimmed)
            break;
    }
    string.erase(idx + 1, string.length());
}
