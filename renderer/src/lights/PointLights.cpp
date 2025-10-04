//
// Created by ghima on 02-10-2025.
//
#include "lights/PointLights.h"
#include "lights/PointLightShadowMap.h"
#include "StaticMesh.h"

namespace rn {
    std::uint32_t PointLights::mCurrentLightSizeCount = 0;
    struct PointLightUBO PointLights::mPointLightUBO{};
    RendererContext *PointLights::mCtx = nullptr;
    List<class PointLightShadowMap *> PointLights::mPointLightShadowMaps = {};
    List<VkDescriptorSet> PointLights::mPointLightShadowDescriptorSets = {};
    VkCommandBuffer PointLights::mShadowCommandBuffer = {};
    VkFence PointLights::renderShadowSceneFence = {};

    PointLights::PointLights(rn::RendererContext *ctx) {
        if (ctx == nullptr) {
            LOG_ERROR("Failed to get the renderer context for the point lights");
            std::exit(EXIT_FAILURE);
        }
        mCtx = ctx;
        ctx->AddPointLight = &PointLights::AddPointLight;
        ctx->UpdateLightInfoPosition = &PointLights::UpdateLightInfoPosition;

        CreatePointLightBuffers();
        BindPointLightDescriptors();
        CreateShadowMapSemaphoreAndAllocateCommandbuffer();

    }

    void PointLights::CreatePointLightBuffers() {
        VkDeviceSize bufferSize = sizeof(PointLightUBO);
        mPointLightsBuffer.resize(mCtx->swapChainImageCount);
        mPointLightsMemory.resize(mCtx->swapChainImageCount);
        for (int i = 0; i < mCtx->swapChainImageCount; i++) {
            Utility::CreateBuffer(*mCtx, mPointLightsBuffer[i], (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
                                  mPointLightsMemory[i],
                                  (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
                                  bufferSize,
                                  "Point Lights Buffer");
        }
    }

    void PointLights::BindPointLightDescriptors() {
        mPointLightDescriptorSets.resize(mCtx->swapChainImageCount);
        List<VkDescriptorSetLayout> layouts(mCtx->swapChainImageCount, mCtx->pointLightLayout);

        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorSetCount = mCtx->swapChainImageCount;
        allocateInfo.descriptorPool = mCtx->pointLightDescriptorPool;
        allocateInfo.pSetLayouts = layouts.data();
        Utility::CheckVulkanError(
                vkAllocateDescriptorSets(mCtx->logicalDevice, &allocateInfo, mPointLightDescriptorSets.data()),
                "Failed to allocate the descriptor sets for the point lights");
        for (int i = 0; i < mCtx->swapChainImageCount; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(PointLightUBO);
            bufferInfo.buffer = mPointLightsBuffer[i];

            VkWriteDescriptorSet writeInfo{};
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.dstBinding = 0;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeInfo.dstSet = mPointLightDescriptorSets[i];
            writeInfo.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(mCtx->logicalDevice, 1, &writeInfo, 0, nullptr);
        }
    }

    void PointLights::BindPointLightShadowDescriptors() {
        vkResetDescriptorPool(mCtx->logicalDevice, mCtx->pointLightShadowPool, 0);
        mPointLightShadowDescriptorSets.resize(mCtx->swapChainImageCount);
        List<VkDescriptorSetLayout> layouts(mCtx->swapChainImageCount, mCtx->pointLightShadowLayout);

        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorSetCount = mCtx->swapChainImageCount;
        allocateInfo.pSetLayouts = layouts.data();
        allocateInfo.descriptorPool = mCtx->pointLightShadowPool;

        vkAllocateDescriptorSets(mCtx->logicalDevice, &allocateInfo, mPointLightShadowDescriptorSets.data());
        for (int i = 0; i < mCtx->swapChainImageCount; i++) {
            List<VkDescriptorImageInfo> imageInfos(MAX_POINT_LIGHTS);
            for (uint32_t j = 0; j < mCurrentLightSizeCount; j++) {
                imageInfos[j].sampler = mPointLightShadowMaps[j]->GetSampler();
                imageInfos[j].imageView = mPointLightShadowMaps[j]->GetCubeImageView();
                imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            for (uint32_t j = mCurrentLightSizeCount; j < MAX_POINT_LIGHTS; j++) {
                // just adding the image view for the first light as this function is called only if at least one light is there
                imageInfos[j].sampler = mPointLightShadowMaps[0]->GetSampler();
                imageInfos[j].imageView = mPointLightShadowMaps[0]->GetCubeImageView();
                imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = mPointLightShadowDescriptorSets[i];
            write.dstBinding = 0;
            write.dstArrayElement = 0;
            write.descriptorCount = MAX_POINT_LIGHTS;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.pImageInfo = imageInfos.data();

            vkUpdateDescriptorSets(mCtx->logicalDevice, 1, &write, 0, nullptr);
        }
    }

    void PointLights::UpdatePointLightBuffers(size_t currentImageIndex) {
        void *data;
        vkMapMemory(mCtx->logicalDevice, mPointLightsMemory[currentImageIndex], 0, sizeof(PointLightUBO), 0, &data);
        memcpy(data, &mPointLightUBO, sizeof(PointLightUBO));
        vkUnmapMemory(mCtx->logicalDevice, mPointLightsMemory[currentImageIndex]);
    }

    std::uint32_t PointLights::AddPointLight(const PointLightInfo &info) {
        // This will return the index for the light added not the size;
        if (mCurrentLightSizeCount > MAX_POINT_LIGHTS) {
            return -1;
        }
        uint32_t indexToAdd = mCurrentLightSizeCount;
        mPointLightUBO.infos[indexToAdd] = info;
        mCurrentLightSizeCount++;
        mPointLightUBO.totalLightCount = mCurrentLightSizeCount;

        PointLightShadowMap *shadowMap = new PointLightShadowMap(mCtx, info, mShadowCommandBuffer);
        mPointLightShadowMaps.push_back(shadowMap);
        BindPointLightShadowDescriptors();

        return indexToAdd;
    }

    PointLights::~PointLights() {
        for (int i = 0; i < mCtx->swapChainImageCount; i++) {
            vkDestroyBuffer(mCtx->logicalDevice, mPointLightsBuffer[i], nullptr);
            vkFreeMemory(mCtx->logicalDevice, mPointLightsMemory[i], nullptr);
        }
    }

    void PointLights::UpdateLightInfoPosition(const glm::vec4 &position, std::uint32_t lightId) {
        mPointLightUBO.infos[lightId].position = position;
    }

    void PointLights::RenderPointLightShadowScene() {
        vkWaitForFences(mCtx->logicalDevice, 1, &renderShadowSceneFence, VK_TRUE, UINT64_MAX);
        vkResetFences(mCtx->logicalDevice, 1, &renderShadowSceneFence);
        vkResetCommandBuffer(mShadowCommandBuffer, 0);

        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(mShadowCommandBuffer, &commandBufferBeginInfo);
        for (int i = 0; i < mPointLightShadowMaps.size(); i++) {
            mPointLightShadowMaps[i]->UpdateLightInfoInShadowMap(mPointLightUBO.infos[i]);
            mPointLightShadowMaps[i]->BeginPointShadowFrame();
            mPointLightShadowMaps[i]->EndFrame();
        }
        vkEndCommandBuffer(mShadowCommandBuffer);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &mPointLightShadowMapSemaphore;
        submitInfo.pCommandBuffers = &mShadowCommandBuffer;

        vkQueueSubmit(mCtx->graphicsQueue, 1, &submitInfo, renderShadowSceneFence);

    }

    void PointLights::CreateShadowMapSemaphoreAndAllocateCommandbuffer() {
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        Utility::CheckVulkanError(
                vkCreateSemaphore(mCtx->logicalDevice, &semaphoreCreateInfo, nullptr, &mPointLightShadowMapSemaphore),
                "Failed to create the semaphore for the point light shadows");
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        Utility::CheckVulkanError(
                vkCreateFence(mCtx->logicalDevice, &fenceCreateInfo, nullptr, &renderShadowSceneFence),
                "Failed  to create the render shadow scene fence for the point lights");
        // Allocating the command buffer;
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandBufferCount = 1;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = mCtx->commandPool;

        Utility::CheckVulkanError(vkAllocateCommandBuffers(mCtx->logicalDevice, &allocateInfo, &mShadowCommandBuffer),
                                  "Failed to allocate the command buffer for the point light shadows");
    }
}