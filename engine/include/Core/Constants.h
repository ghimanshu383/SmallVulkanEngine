//
// Created by ghima on 27-08-2025.
//

#ifndef SMALLVKENGINE_CONSTANTS_H
#define SMALLVKENGINE_CONSTANTS_H

#include "precomp.h"

namespace vk {
    template<typename T>
    using List = std::vector<T>;
    class Constants {
    public:
        static std::uint32_t WINDOW_WIDTH;
        static std::uint32_t WINDOW_HEIGHT;

        static void ParseObjectString(std::string & string, List<std::string>& substring, char token);
    };
}
#endif //SMALLVKENGINE_CONSTANTS_H
