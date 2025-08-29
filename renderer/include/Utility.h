//
// Created by ghima on 27-08-2025.
//

#ifndef SMALLVKENGINE_UTILITY_H
#define SMALLVKENGINE_UTILITY_H

#include "precomp.h"

namespace rn {
#define LOG_INFO(M, ...) spdlog::info(M, ##__VA_ARGS__)
#define LOG_ERROR(M, ...) spdlog::error(M, ##__VA_ARGS__)
#define LOG_WARN(M, ...) spdlog::warn(M, ##__VA_ARGS__)
    template<typename T>
    using List = std::vector<T>;

    class Utility {
    public:
        static void ReadFileBinary(const char *fileName, List<std::uint8_t> &buffer) {
            std::ifstream inputStream{fileName, std::ios::binary | std::ios::ate};
            if (!inputStream) {
                LOG_ERROR("Failed to Read the Shader Module File {}", fileName);
                std::exit(EXIT_FAILURE);
            }
            uint32_t fileSize = inputStream.tellg();
            buffer.resize(fileSize);
            inputStream.seekg(0);
            inputStream.read(reinterpret_cast<char *>(buffer.data()), fileSize);
            inputStream.close();
        }
    };
}
#endif //SMALLVKENGINE_UTILITY_H
