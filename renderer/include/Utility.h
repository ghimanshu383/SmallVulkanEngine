//
// Created by ghima on 27-08-2025.
//

#ifndef SMALLVKENGINE_UTILITY_H
#define SMALLVKENGINE_UTILITY_H

#define STBI_NO_SIMD

#include "stb_image.h"
#include "precomp.h"

namespace rn {
#define LOG_INFO(M, ...) spdlog::info(M, ##__VA_ARGS__)
#define LOG_ERROR(M, ...) spdlog::error(M, ##__VA_ARGS__)
#define LOG_WARN(M, ...) spdlog::warn(M, ##__VA_ARGS__)
    template<typename T>
    using List = std::vector<T>;
    template<typename T, typename R, typename S>
    using Map = std::unordered_map<T, R, S>;
    struct Vertex {
        glm::vec3 pos;
        glm::vec4 color;
        glm::vec2 uv;
        glm::vec3 normals;
    };
    struct alignas(16) ViewProjection {
        glm::mat4 projection;
        glm::mat4 view;
    };
    struct ModelUBO {
        glm::mat4 model;
    };

    struct alignas(16) OmniDirectionalInfo {
        glm::mat4 projection;
        glm::mat4 view;
        glm::vec4 position;
        glm::vec4 color;
        glm::vec4 intensities;
    };

    struct RendererContext {
        size_t swapChainImageCount;
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
        VkCommandPool commandPool;
        VkQueue graphicsQueue;
        VkQueue presentationQueue;
        VkDescriptorPool samplerDescriptorPool;
        VkDescriptorSetLayout samplerDescriptorSetLayout;
        VkSampler textureSampler;
        VkDescriptorSetLayout lightsLayout;
        VkDescriptorPool lightsDescriptorPool;
        VkDescriptorSet shadowDescriptorSet;

        VkSwapchainKHR swapchain;
        VkFormat swapChainFormat;
        List<VkImageView> *swapChainImageViews;
        VkExtent2D windowExtents;
        size_t currentImageIndex;
        List<VkDescriptorSet> *imguiViewPortDescriptors;

        void (*RegisterMesh)(std::string &id, class StaticMesh *);

        Map<std::string, class StaticMesh *, std::hash<std::string>> *(*GetSceneObjectMap)();

        class Texture *(*RegisterTexture)(std::string &texturePathId);

        void (*UpdateViewAndProjectionMatrix)(ViewProjection &&viewProjection);

        void (*SetUpAsDirectionalLight)(class OmniDirectionalLight *directionalLight);
    };


    class Utility {
    public:
        static uint32_t MAX_OBJECTS;

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

        static std::uint8_t *LoadTextureImage(const char *fileName, int &width, int &height, VkDeviceSize &imageSize);

        static void
        CopyBufferToImage(RendererContext &ctx, VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height,
                          VkImageAspectFlags aspectFlags);

        static void
        TransitionImageLayout(RendererContext ctx, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                              VkImageAspectFlags aspectFlags);

        static VkImage
        CreateImage(std::string &&imageName, VkPhysicalDevice physicalDevice, VkDevice logicalDevice,
                    unsigned int width, unsigned int height,
                    VkFormat format,
                    VkImageTiling imageTiling,
                    unsigned int imageUsageFlags, unsigned int memoryPropertyFlags,
                    VkDeviceMemory &memory);

        static void CreateImageView(VkDevice logicalDevice, VkImage &image, VkFormat format, VkImageView &imageView,
                                    unsigned int imageAspect);

        static VkShaderModule CreateShaderModule(VkDevice logicalDevice, const char *filePath);
    };
}
#endif //SMALLVKENGINE_UTILITY_H
