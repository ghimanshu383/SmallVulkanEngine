//
// Created by ghima on 11-10-2025.
//
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>
#include "Utility.h"
#include "pbr/PbrMaterials.h"
#include "pbr/PbrTexture.h"
#include "lights/PointLights.h"
#include "StaticMesh.h"

namespace rn {
    PbrMaterial::PbrMaterial(RendererContext *ctx, VkDescriptorSetLayout setLayout,
                             const std::string &baseTextureLoc) : mCtx{ctx}, albedo{nullptr},
                                                                  normal{nullptr},
                                                                  metallic{nullptr},
                                                                  roughness{nullptr}, ao{nullptr},
                                                                  mSetLayout{setLayout} {
        albedo = new PbrTexture(ctx, baseTextureLoc + "\\albedo.png");
        normal = new PbrTexture(ctx, baseTextureLoc + "\\normal.png");
        metallic = new PbrTexture(ctx, baseTextureLoc + "\\metallic.png");
        roughness = new PbrTexture(ctx, baseTextureLoc + "\\roughness.png");
        ao = new PbrTexture(ctx, baseTextureLoc + "\\ao.png");

        CreateDescriptorPoolAndSets();
        CreateUniformBuffersAndBindToDescriptorSets();

    }

    void PbrMaterial::CreateDescriptorPoolAndSets() {
        // Creating a descriptor Pool for the sets;
        VkDescriptorPoolSize uniformPoolSize{};
        uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformPoolSize.descriptorCount = 3;

        VkDescriptorPoolSize samplerPoolSize{};
        samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerPoolSize.descriptorCount = 5;

        List<VkDescriptorPoolSize> poolSizes{uniformPoolSize, samplerPoolSize};

        VkDescriptorPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolCreateInfo.maxSets = 1;
        poolCreateInfo.poolSizeCount = poolSizes.size();
        poolCreateInfo.pPoolSizes = poolSizes.data();

        Utility::CheckVulkanError(
                vkCreateDescriptorPool(mCtx->logicalDevice, &poolCreateInfo, nullptr, &mDescriptorPool),
                "Failed to create the descriptor pool for the pbr ");
        // Allocating the descriptor sets and binding
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pSetLayouts = &mSetLayout;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.descriptorPool = mDescriptorPool;
        allocateInfo.pNext = nullptr;

        Utility::CheckVulkanError(
                vkAllocateDescriptorSets(mCtx->logicalDevice, &allocateInfo, &mDescriptorSet),
                "Failed to allocate descriptor set for PBR material");
    }

    void PbrMaterial::CreateUniformBuffersAndBindToDescriptorSets() {
        Utility::CreateBuffer(*mCtx, mViewProjectionBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mViewProjectionMemory,
                              (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                              sizeof(CameraUBO), "Pbr View Projection Buffer");
        Utility::CreateBuffer(*mCtx, mDiffuseLightBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mDiffuseLightMemory,
                              (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                              sizeof(OmniDirectionalInfo), "Pbr Diffuse Light Buffer");
        Utility::CreateBuffer(*mCtx, mPointLightBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mPointLightMemory,
                              (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                              sizeof(PointLightUBO), "Pbr Point light Buffer");

        VkDescriptorBufferInfo viewProjectionBufferInfo{};
        viewProjectionBufferInfo.offset = 0;
        viewProjectionBufferInfo.range = sizeof(CameraUBO);
        viewProjectionBufferInfo.buffer = mViewProjectionBuffer;

        VkDescriptorBufferInfo diffuseLightBufferInfo{};
        diffuseLightBufferInfo.offset = 0;
        diffuseLightBufferInfo.range = sizeof(OmniDirectionalInfo);
        diffuseLightBufferInfo.buffer = mDiffuseLightBuffer;

        VkDescriptorBufferInfo pointLightBufferInfo{};
        pointLightBufferInfo.offset = 0;
        pointLightBufferInfo.range = sizeof(PointLightUBO);
        pointLightBufferInfo.buffer = mPointLightBuffer;

        VkWriteDescriptorSet viewProjectionBufferWrite{};
        viewProjectionBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        viewProjectionBufferWrite.descriptorCount = 1;
        viewProjectionBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        viewProjectionBufferWrite.dstBinding = 0;
        viewProjectionBufferWrite.dstArrayElement = 0;
        viewProjectionBufferWrite.dstSet = mDescriptorSet;
        viewProjectionBufferWrite.pBufferInfo = &viewProjectionBufferInfo;

        VkWriteDescriptorSet diffuseLightBufferWrite{};
        diffuseLightBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        diffuseLightBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        diffuseLightBufferWrite.descriptorCount = 1;
        diffuseLightBufferWrite.dstBinding = 1;
        diffuseLightBufferWrite.dstArrayElement = 0;
        diffuseLightBufferWrite.dstSet = mDescriptorSet;
        diffuseLightBufferWrite.pBufferInfo = &diffuseLightBufferInfo;

        VkWriteDescriptorSet pointLightBufferWrite{};
        pointLightBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        pointLightBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pointLightBufferWrite.descriptorCount = 1;
        pointLightBufferWrite.dstBinding = 2;
        pointLightBufferWrite.dstArrayElement = 0;
        pointLightBufferWrite.dstSet = mDescriptorSet;
        pointLightBufferWrite.pBufferInfo = &pointLightBufferInfo;

        VkWriteDescriptorSet albedoWrite = albedo->GetWriteSamplerDescriptorForTexture(mDescriptorSet, 3, 0);
        VkWriteDescriptorSet normalWrite = normal->GetWriteSamplerDescriptorForTexture(mDescriptorSet, 3, 1);
        VkWriteDescriptorSet metallicWrite = metallic->GetWriteSamplerDescriptorForTexture(mDescriptorSet, 3, 2);
        VkWriteDescriptorSet roughnessWrite = roughness->GetWriteSamplerDescriptorForTexture(mDescriptorSet, 3, 3);
        VkWriteDescriptorSet aoWrite = ao->GetWriteSamplerDescriptorForTexture(mDescriptorSet, 3, 4);


        List<VkWriteDescriptorSet> writes{viewProjectionBufferWrite, diffuseLightBufferWrite, pointLightBufferWrite,
                                          albedoWrite, normalWrite, metallicWrite, roughnessWrite, aoWrite};
        vkUpdateDescriptorSets(mCtx->logicalDevice, writes.size(), writes.data(), 0, nullptr);
    }

    void PbrMaterial::UpdateUniformBuffersFromGraphicsContext() {

        // updating the view projection Buffers
        void *data;
        CameraUBO cameraUbo{mCtx->GetViewProjectionMatrix()->projection, mCtx->GetViewProjectionMatrix()->view,
                            {mCtx->cameraPosition, 1.0}};
        vkMapMemory(mCtx->logicalDevice, mViewProjectionMemory, 0, sizeof(CameraUBO), 0, &data);
        memcpy(data, &cameraUbo, sizeof(CameraUBO));
        vkUnmapMemory(mCtx->logicalDevice, mViewProjectionMemory);

        // updating the diffuse light buffers;
        vkMapMemory(mCtx->logicalDevice, mDiffuseLightMemory, 0, sizeof(OmniDirectionalInfo), 0, &data);
        memcpy(data, &mCtx->GetDirectionalLightInfo(), sizeof(OmniDirectionalInfo));
        vkUnmapMemory(mCtx->logicalDevice, mDiffuseLightMemory);

        // Updating the point light buffers;
        vkMapMemory(mCtx->logicalDevice, mPointLightMemory, 0, sizeof(PointLightUBO), 0, &data);
        memcpy(data, &mCtx->pointLight->GetPointLightUBO(), sizeof(PointLightUBO));
        vkUnmapMemory(mCtx->logicalDevice, mPointLightMemory);

    }

    PbrMaterial::~PbrMaterial() {
        vkDestroyDescriptorPool(mCtx->logicalDevice, mDescriptorPool, nullptr);
        vkDestroyBuffer(mCtx->logicalDevice, mPointLightBuffer, nullptr);
        vkDestroyBuffer(mCtx->logicalDevice, mDiffuseLightBuffer, nullptr);
        vkDestroyBuffer(mCtx->logicalDevice, mViewProjectionBuffer, nullptr);

        vkFreeMemory(mCtx->logicalDevice, mPointLightMemory, nullptr);
        vkFreeMemory(mCtx->logicalDevice, mDiffuseLightMemory, nullptr);
        vkFreeMemory(mCtx->logicalDevice, mViewProjectionMemory, nullptr);

        delete albedo;
        delete normal;
        delete metallic;
        delete roughness;
        delete ao;
    }

}