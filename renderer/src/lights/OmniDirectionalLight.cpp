//
// Created by ghima on 10-09-2025.
//
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "lights/OmniDirectionalLight.h"
#include "lights/ShadowMap.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RIGHT_HANDED

namespace rn {
    OmniDirectionalLight::OmniDirectionalLight(const std::string &id, rn::RendererContext *ctx,
                                               const rn::OmniDirectionalInfo &info) : mCtx{ctx}, mLightInfo(info),
                                                                                      mShadowMap{nullptr} {
        CreateLightBuffers();
        CreateLightDescriptorSets();
        ComputeViewProjection();
        CreateShadowMap();
    }

    OmniDirectionalLight::~OmniDirectionalLight() {
        for (size_t i = 0; i < mLightBuffer.size(); i++) {
            vkDestroyBuffer(mCtx->logicalDevice, mLightBuffer[i], nullptr);
            vkFreeMemory(mCtx->logicalDevice, mLightBufferMemory[i], nullptr);
        }
        delete mShadowMap;
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

    void OmniDirectionalLight::CreateShadowMap() {
        mShadowMap = new ShadowMap(mCtx, this, 800, 800, mCtx->GetSceneObjectMap());
        mShadowMap->Init();
    }

    ShadowMap *OmniDirectionalLight::GetShadowMap() const {
        return mShadowMap;
    }

    void OmniDirectionalLight::ComputeViewProjection() {
        float orthoSize = 5.0f; // Adjust to cover your scene
        mLightInfo.projection = glm::orthoZO(-orthoSize, orthoSize, -orthoSize, orthoSize, .1f, 20.f);
//        mLightInfo.projection = glm::perspective(glm::radians(45.0f),
//                                                 (float) mCtx->windowExtents.width / (float) mCtx->windowExtents.height,
//                                                 .1f, 100.f);
        mLightInfo.projection[1][1] *= -1;
        glm::vec3 lightPos = glm::vec3(mLightInfo.position);
        mLightInfo.view = glm::lookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        mViewProjection.projection = mLightInfo.projection;
        mViewProjection.view = mLightInfo.view;
    }

    ViewProjection &OmniDirectionalLight::GetLightViewProjection() {
        ComputeViewProjection();
        return mViewProjection;
    }
}