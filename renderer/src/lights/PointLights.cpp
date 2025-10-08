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
    List<VkCommandBuffer> PointLights::mShadowCommandBuffer = {};
    VkFence PointLights::renderShadowSceneFence = {};
    VkSampler  PointLights::mDummyShadowSampler{};
    VkImageView PointLights::mDummyShadowImageview{};
    List<std::thread> PointLights::mShadowMapThreads{};

    PointLights::PointLights(rn::RendererContext *ctx) {
        if (ctx == nullptr) {
            LOG_ERROR("Failed to get the renderer context for the point lights");
            std::exit(EXIT_FAILURE);
        }
        VkCommandBuffer commandBuffer{};
        mShadowCommandBuffer = {MAX_POINT_LIGHTS, commandBuffer};
        mCtx = ctx;
        ctx->AddPointLight = &PointLights::AddPointLight;
        ctx->UpdateLightInfoPosition = &PointLights::UpdateLightInfoPosition;

        CreatePointLightBuffers();
        BindPointLightDescriptors();
        CreateShadowMapSemaphoreAndAllocateCommandbuffer();
        CreateDummyShadowBindingContext();
        BindPointLightShadowDescriptors();

    }

    PointLights::~PointLights() {
        for (int i = 0; i < mCtx->swapChainImageCount; i++) {
            vkDestroyBuffer(mCtx->logicalDevice, mPointLightsBuffer[i], nullptr);
            vkFreeMemory(mCtx->logicalDevice, mPointLightsMemory[i], nullptr);
        }
        vkDestroySampler(mCtx->logicalDevice, mDummyShadowSampler, nullptr);
        vkDestroyImageView(mCtx->logicalDevice, mDummyShadowImageview, nullptr);
        vkDestroyImage(mCtx->logicalDevice, mDummyShadowImage, nullptr);
        vkFreeMemory(mCtx->logicalDevice, mDummyImageMemory, nullptr);

        vkDestroyFence(mCtx->logicalDevice, renderShadowSceneFence, nullptr);
        vkDestroySemaphore(mCtx->logicalDevice, mPointLightShadowMapSemaphore, nullptr);
        for (const PointLightShadowMap *shadowMap: mPointLightShadowMaps) {
            delete shadowMap;
        }
        for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
            vkDestroyCommandPool(mCtx->logicalDevice, mThreadedCommandPools[i], nullptr);
        }
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
                imageInfos[j].sampler = mDummyShadowSampler;
                imageInfos[j].imageView = mDummyShadowImageview;
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

        PointLightShadowMap *shadowMap = new PointLightShadowMap(mCtx, info);
        mPointLightShadowMaps.push_back(shadowMap);
        BindPointLightShadowDescriptors();

        return indexToAdd;
    }

    void PointLights::UpdateLightInfoPosition(const glm::vec4 &position, std::uint32_t lightId) {
        mPointLightUBO.infos[lightId].position = position;
    }

    void PointLights::RenderPointLightShadowScene() {
        List<VkCommandBuffer> activeCommandBuffer{};
        vkWaitForFences(mCtx->logicalDevice, 1, &renderShadowSceneFence, VK_TRUE, UINT64_MAX);
        vkResetFences(mCtx->logicalDevice, 1, &renderShadowSceneFence);


        for (int i = 0; i < mPointLightShadowMaps.size(); i++) {
            mShadowMapThreads.emplace_back([&, i]() -> void {
                vkResetCommandBuffer(mShadowCommandBuffer[i], 0);
                VkCommandBufferBeginInfo commandBufferBeginInfo{};
                commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                vkBeginCommandBuffer(mShadowCommandBuffer[i], &commandBufferBeginInfo);
                mPointLightShadowMaps[i]->UpdateLightInfoInShadowMap(mPointLightUBO.infos[i]);
                mPointLightShadowMaps[i]->BeginPointShadowFrame(mShadowCommandBuffer[i]);
                mPointLightShadowMaps[i]->EndFrame(mShadowCommandBuffer[i]);
                vkEndCommandBuffer(mShadowCommandBuffer[i]);
                {
                    std::lock_guard<std::mutex> lockGuard{mutex_};
                    activeCommandBuffer.emplace_back(mShadowCommandBuffer[i]);
                }
            });
        }

        for (auto &thread: mShadowMapThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &mPointLightShadowMapSemaphore;
        submitInfo.commandBufferCount = activeCommandBuffer.size();
        submitInfo.pCommandBuffers = activeCommandBuffer.data();

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

        VkCommandPool pool{};
        mThreadedCommandPools = {MAX_POINT_LIGHTS, pool};
        // Creating the command Pool For Each Thread;
        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = mCtx->graphicsQueueIndex;
        for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
            Utility::CheckVulkanError(
                    vkCreateCommandPool(mCtx->logicalDevice, &commandPoolCreateInfo, nullptr,
                                        &mThreadedCommandPools[i]),
                    "Failed to create the threaded command pool for point light shadows");
        }

        // Allocating the command buffer;

        for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
            VkCommandBufferAllocateInfo allocateInfo{};
            allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocateInfo.commandBufferCount = 1;
            allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocateInfo.commandPool = mThreadedCommandPools[i];

            Utility::CheckVulkanError(
                    vkAllocateCommandBuffers(mCtx->logicalDevice, &allocateInfo, &mShadowCommandBuffer[i]),
                    "Failed to allocate the command buffer for the point light shadows");
        }
    }

    void PointLights::CreateDummyShadowBindingContext() {
        mDummyShadowImage = Utility::CreateImage("Point Light Shadow Image", mCtx->physicalDevice, mCtx->logicalDevice,
                                                 SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
                                                 VK_FORMAT_R32_SFLOAT,
                                                 VK_IMAGE_TILING_OPTIMAL,
                                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                 mDummyImageMemory, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

        Utility::CreateImageView(mCtx->logicalDevice, mDummyShadowImage, VK_FORMAT_R32_SFLOAT, mDummyShadowImageview,
                                 VK_IMAGE_ASPECT_COLOR_BIT, 0, 6, VK_IMAGE_VIEW_TYPE_CUBE);
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerCreateInfo.maxAnisotropy = 1.0;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;

        Utility::CheckVulkanError(
                vkCreateSampler(mCtx->logicalDevice, &samplerCreateInfo, nullptr, &mDummyShadowSampler),
                "Failed to create th dummy sampler for the piont lights");
    }
}