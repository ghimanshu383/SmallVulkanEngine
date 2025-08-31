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
    template<typename T, typename R, typename S>
    using Map = std::unordered_map<T, R, S>;

    struct RendererContext {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
        VkCommandPool commandPool;
        VkQueue graphicsQueue;
        VkQueue presentationQueue;

        void (*RegisterMesh)(std::string &id, class StaticMesh *);
    };

    struct Vertex {
        glm::vec3 pos;
        glm::vec4 color;
    };

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

        static void CheckVulkanError(VkResult result, const char *message) {
            if (result != VK_SUCCESS) {
                LOG_ERROR("VULKAN GRAPHICS ERROR : %s", message);
                std::exit(EXIT_FAILURE);
            }
        }

        static void CreateBuffer(RendererContext &ctx, VkBuffer &buffer, VkBufferUsageFlags usageFlags,
                                 VkDeviceMemory &bufferMemory, VkMemoryPropertyFlags bufferMemoryFlags,
                                 VkDeviceSize requiredBufferSize, std::string &bufferName);

        static std::uint32_t FindMemoryIndices(VkPhysicalDevice physicalDevice, uint32_t allowedTypes,
                                               VkMemoryPropertyFlags requiredMemoryFlags, std::string &bufferName);

        static VkCommandBuffer BeginCommandBuffer(RendererContext ctx);

        static void SubmitCommandBuffer(RendererContext &ctx, VkCommandBuffer &commandBuffer);

        static void CopyBuffers(RendererContext &ctx, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);
    };
}
#endif //SMALLVKENGINE_UTILITY_H
