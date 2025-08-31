//
// Created by ghima on 30-08-2025.
//
#include "Utility.h"

namespace rn {
    std::uint32_t Utility::FindMemoryIndices(VkPhysicalDevice physicalDevice, std::uint32_t requiredIndex,
                                             VkMemoryPropertyFlags requiredMemoryFlags, std::string &bufferName) {
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
                               VkDeviceSize requiredBufferSize, std::string &bufferName) {
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
}