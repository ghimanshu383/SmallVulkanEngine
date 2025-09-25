//
// Created by ghima on 27-08-2025.
//
#include "Core/Constants.h"

namespace vk {
    std::uint32_t Constants::WINDOW_WIDTH = 800;
    std::uint32_t Constants::WINDOW_HEIGHT = 600;

    std::uint32_t Constants::MAX_LOGS = 100;

    void Constants::ParseObjectString(std::string &string, List<std::string> &subString, char token) {
        size_t start = 0;
        size_t end = 0;
        while (end != std::string::npos) {
            end = string.find(token, start);
            if (end - start > 0) {
                subString.push_back(string.substr(start, (end - start)));
            }
            start = end + 1;
        }
    }
}