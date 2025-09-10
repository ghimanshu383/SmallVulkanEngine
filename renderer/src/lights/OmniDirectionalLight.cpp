//
// Created by ghima on 10-09-2025.
//
#include "lights/OmniDirectionalLight.h"

namespace rn {
    OmniDirectionalLight::OmniDirectionalLight(const std::string &id, rn::RendererContext *ctx,
                                               const rn::OmniDirectionalInfo &info) : mCtx{ctx}, mLightInfo(info) {
        CreateLightBuffers();
        CreateLightDescriptorSets();
    }

    OmniDirectionalLight::~OmniDirectionalLight() {
        for (size_t i = 0; i < mLightBuffer.size(); i++) {
            vkDestroyBuffer(mCtx->logicalDevice, mLightBuffer[i], nullptr);
            vkFreeMemory(mCtx->logicalDevice, mLightBufferMemory[i], nullptr);
        }
    }

    void OmniDirectionalLight::CreateLightBuffers() {
        VkDeviceSize bufferSize = sizeof(OmniDirectionalInfo);
        mLightBuffer.resize(mCtx->swapChainImageCount);
        mLightBufferMemory.resize(mCtx->swapChainImageCount);
        for (size_t i = 0; i < mCtx->swapChainImageCount; i++) {
            std::string bufferName = "Omni Directional Buffer";
            Utility::CreateBuffer(*mCtx, mLightBuffer[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mLightBufferMemory[i],
                                  (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                                  bufferSize, bufferName);
        }
    }

    void OmniDirectionalLight::CreateLightDescriptorSets() {
        mLightDescriptorSets.resize(mCtx->swapChainImageCount);
        List<VkDescriptorSetLayout> lightsLayouts(mCtx->swapChainImageCount,
                                                  mCtx->lightsLayout);

        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorSetCount = mLightDescriptorSets.size();
        allocateInfo.pSetLayouts = lightsLayouts.data();
        allocateInfo.descriptorPool = mCtx->lightsDescriptorPool;

        Utility::CheckVulkanError(
                vkAllocateDescriptorSets(mCtx->logicalDevice, &allocateInfo, mLightDescriptorSets.data()),
                "Failed to allocate the descriptor sets for lights");
        for (size_t i = 0; i < mCtx->swapChainImageCount; i++) {
            VkDescriptorBufferInfo lightBufferInfo{};
            lightBufferInfo.offset = 0;
            lightBufferInfo.buffer = mLightBuffer[i];
            lightBufferInfo.range = sizeof(OmniDirectionalInfo);

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.dstSet = mLightDescriptorSets[i];
            writeDescriptorSet.pBufferInfo = &lightBufferInfo;

            List<VkWriteDescriptorSet> writeInfo{writeDescriptorSet};
            vkUpdateDescriptorSets(mCtx->logicalDevice, writeInfo.size(), writeInfo.data(), 0, nullptr);
        }
    }

    void OmniDirectionalLight::UpdateLightDescriptorSet(size_t currentImageIndex) {
        void *data;
        vkMapMemory(mCtx->logicalDevice, mLightBufferMemory[currentImageIndex], 0, sizeof(OmniDirectionalInfo), 0,
                    &data);
        memcpy(data, &mLightInfo, sizeof(OmniDirectionalInfo));
        vkUnmapMemory(mCtx->logicalDevice, mLightBufferMemory[currentImageIndex]);
    }
}