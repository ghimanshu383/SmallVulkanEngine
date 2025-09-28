//
// Created by ghima on 30-08-2025.
//
#define STB_IMAGE_IMPLEMENTATION

#include "Utility.h"

namespace rn {
    std::uint32_t Utility::MAX_OBJECTS = 1000;

    std::uint32_t Utility::FindMemoryIndices(VkPhysicalDevice physicalDevice, std::uint32_t requiredIndex,
                                             VkMemoryPropertyFlags requiredMemoryFlags, const std::string &bufferName) {
        VkPhysicalDeviceMemoryProperties memoryProperties{};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
        for (size_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((requiredIndex & (1 << i)) &&
                (memoryProperties.memoryTypes[i].propertyFlags & requiredMemoryFlags) == requiredMemoryFlags) {
                return i;
            }
        }
        LOG_ERROR("Failed to Get the Required Memory Index {}", bufferName.c_str());
        std::exit(EXIT_FAILURE);
    }

    void Utility::CreateBuffer(rn::RendererContext &ctx, VkBuffer &buffer, VkBufferUsageFlags usageFlags,
                               VkDeviceMemory &bufferMemory, VkMemoryPropertyFlags bufferMemoryFlags,
                               VkDeviceSize requiredBufferSize, const std::string &bufferName) {
        VkBufferCreateInfo createBufferInfo{};
        createBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createBufferInfo.size = requiredBufferSize;
        createBufferInfo.usage = usageFlags;
        createBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        std::string errorMessage = "Failed to create " + bufferName;
        CheckVulkanError(vkCreateBuffer(ctx.logicalDevice, &createBufferInfo, nullptr, &buffer),
                         errorMessage.c_str());

        VkMemoryRequirements bufferMemoryRequirements{};
        vkGetBufferMemoryRequirements(ctx.logicalDevice, buffer, &bufferMemoryRequirements);

        VkMemoryAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = bufferMemoryRequirements.size;
        allocateInfo.memoryTypeIndex = FindMemoryIndices(ctx.physicalDevice, bufferMemoryRequirements.memoryTypeBits,
                                                         bufferMemoryFlags, bufferName);

        CheckVulkanError(vkAllocateMemory(ctx.logicalDevice, &allocateInfo, nullptr, &bufferMemory),
                         "Failed to allocate the memory for the buffer");
        vkBindBufferMemory(ctx.logicalDevice, buffer, bufferMemory, 0);
    }

    VkCommandBuffer Utility::BeginCommandBuffer(rn::RendererContext ctx) {
        VkCommandBuffer commandBuffer{};
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = ctx.commandPool;
        allocateInfo.commandBufferCount = 1;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        CheckVulkanError(vkAllocateCommandBuffers(ctx.logicalDevice, &allocateInfo, &commandBuffer),
                         "Failed to allocate the command buffer");
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        CheckVulkanError(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Failed to begin the command buffer");

        return commandBuffer;
    }

    void Utility::SubmitCommandBuffer(rn::RendererContext &ctx, VkCommandBuffer &commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkFence submitFence{};
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        vkCreateFence(ctx.logicalDevice, &fenceCreateInfo, nullptr, &submitFence);

        vkQueueSubmit(ctx.graphicsQueue, 1, &submitInfo, submitFence);
        vkWaitForFences(ctx.logicalDevice, 1, &submitFence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(ctx.logicalDevice, submitFence, nullptr);
        vkFreeCommandBuffers(ctx.logicalDevice, ctx.commandPool, 1, &commandBuffer);
    }

    void Utility::CopyBuffers(rn::RendererContext &ctx, VkBuffer srcBuffer, VkBuffer dstBuffer,
                              VkDeviceSize bufferSize) {
        VkCommandBuffer commandBuffer = BeginCommandBuffer(ctx);
        VkBufferCopy region{};
        region.size = bufferSize;
        region.dstOffset = 0;
        region.srcOffset = 0;

        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &region);
        SubmitCommandBuffer(ctx, commandBuffer);
    }

    std::uint8_t *Utility::LoadTextureImage(const char *fileName, int &width, int &height, VkDeviceSize &imageSize) {
        int channel;
        std::uint8_t *imageData = stbi_load(fileName, &width, &height, &channel, STBI_rgb_alpha);
        if (imageData == nullptr) {
            LOG_WARN("Failed to load the texture Image From File... Setting the Default Texture Image");
            // Write the Logic to load the default Texture;
        }
        imageSize = width * height * 4;
        return imageData;
    }

    VkImage Utility::CreateImage(std::string &&imageName, VkPhysicalDevice physicalDevice, VkDevice logicalDevice,
                                 uint32_t width, uint32_t height,
                                 VkFormat format,
                                 VkImageTiling imageTiling,
                                 VkImageUsageFlags imageUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
                                 VkDeviceMemory &memory) {
        VkImageCreateInfo depthImageCreateInfo{};
        depthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        depthImageCreateInfo.format = format;
        depthImageCreateInfo.extent.width = width;
        depthImageCreateInfo.extent.height = height;
        depthImageCreateInfo.tiling = imageTiling;
        depthImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthImageCreateInfo.extent.depth = 1;
        depthImageCreateInfo.arrayLayers = 1;
        depthImageCreateInfo.mipLevels = 1;
        depthImageCreateInfo.usage = imageUsageFlags;
        depthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        depthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        depthImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        std::string imageCreationErrorMessage = "Failed to create the Image " + imageName;
        VkImage image{};
        Utility::CheckVulkanError(vkCreateImage(logicalDevice, &depthImageCreateInfo, nullptr, &image),
                                  imageCreationErrorMessage.c_str());

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(logicalDevice, image, &memoryRequirements);
        VkMemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

        memoryAllocateInfo.memoryTypeIndex = Utility::FindMemoryIndices(physicalDevice,
                                                                        memoryRequirements.memoryTypeBits,
                                                                        memoryPropertyFlags, imageName);
        memoryAllocateInfo.allocationSize = memoryRequirements.size;

        Utility::CheckVulkanError(vkAllocateMemory(logicalDevice, &memoryAllocateInfo, nullptr, &memory),
                                  "Failed to allocate memory to Image");


        vkBindImageMemory(logicalDevice, image, memory, 0);
        return image;

    }

    void
    Utility::CreateImageView(VkDevice logicalDevice, VkImage &image, VkFormat format, VkImageView &imageView,
                             VkImageAspectFlags imageAspect) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.format = format;
        imageViewCreateInfo.image = image;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = imageAspect;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;

        Utility::CheckVulkanError(vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &imageView),
                                  "Failed to create the image View");
    }

    void Utility::CopyBufferToImage(RendererContext &ctx, VkBuffer srcBuffer, VkImage dstImage, uint32_t width,
                                    uint32_t height, VkImageAspectFlags aspectFlags) {
        VkCommandBuffer commandBuffer = BeginCommandBuffer(ctx);
        VkBufferImageCopy imageCopy{};
        imageCopy.imageExtent = {width, height, 1};
        imageCopy.imageSubresource.aspectMask = aspectFlags;
        imageCopy.imageSubresource.layerCount = 1;
        imageCopy.imageSubresource.mipLevel = 0;
        imageCopy.imageOffset = {0, 0};
        imageCopy.bufferRowLength = 0;
        imageCopy.bufferOffset = 0;
        imageCopy.bufferImageHeight = 0;

        vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
        SubmitCommandBuffer(ctx, commandBuffer);
    }

    void Utility::TransitionImageLayout(RendererContext ctx, VkImage image, VkImageLayout oldLayout,
                                        VkImageLayout newLayout, VkImageAspectFlags aspectFlags) {
        VkCommandBuffer commandBuffer = BeginCommandBuffer(ctx);
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.oldLayout = oldLayout;
        imageMemoryBarrier.newLayout = newLayout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.subresourceRange.aspectMask = aspectFlags;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;

        VkPipelineStageFlags srcFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags dstFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            srcFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        vkCmdPipelineBarrier(commandBuffer, srcFlags, dstFlags, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &imageMemoryBarrier);
        SubmitCommandBuffer(ctx, commandBuffer);
    }

    VkShaderModule Utility::CreateShaderModule(VkDevice logicalDevice, const char *filePath) {
        VkShaderModule module{};
        List<uint8_t> moduleCode{};
        VkShaderModuleCreateInfo moduleCreateInfo{};
        Utility::ReadFileBinary(filePath, moduleCode);

        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.codeSize = moduleCode.size();
        moduleCreateInfo.pCode = reinterpret_cast<std::uint32_t *>(moduleCode.data());

        Utility::CheckVulkanError(vkCreateShaderModule(logicalDevice, &moduleCreateInfo, nullptr, &module),
                                  "Failed to create the Shader Module");
        return module;
    }
}