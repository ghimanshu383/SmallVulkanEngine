//
// Created by ghima on 02-10-2025.
//
#include "lights/PointLights.h"
#include "StaticMesh.h"

namespace rn {
    std::uint32_t PointLights::mCurrentLightSizeCount = 0;
    struct PointLightUBO PointLights::mPointLightUBO{};

    PointLights::PointLights(rn::RendererContext *ctx) : mCtx{ctx} {
        CreatePointLightBuffers();
        BindPointLightDescriptors();
        ctx->AddPointLight = &PointLights::AddPointLight;
        ctx->UpdateLightInfoPosition = &PointLights::UpdateLightInfoPosition;
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
}