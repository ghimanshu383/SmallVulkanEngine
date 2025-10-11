//
// Created by ghima on 11-10-2025.
//
#include "utility.h"
#include "pbr/PbrTexture.h"

namespace rn {

    PbrTexture::PbrTexture(RendererContext *ctx, const std::string &fileName) : mCtx{ctx} {
        LoadTexture(fileName);
    }

    void PbrTexture::LoadTexture(const std::string &fileName) {
        VkBuffer stagingBuffer{};
        VkDeviceMemory stagingBufferMemory{};
        int width, height;
        VkDeviceSize imageSize;
        uint8_t *imageData = Utility::LoadTextureImage(fileName.c_str(), width, height, imageSize);
        // Creating the data staging Buffer;
        Utility::CreateBuffer(*mCtx, stagingBuffer, (VK_BUFFER_USAGE_TRANSFER_SRC_BIT), stagingBufferMemory,
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, imageSize,
                              "Pbr Texture Staging Buffer");
        void *data;
        vkMapMemory(mCtx->logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, imageData, imageSize);
        vkUnmapMemory(mCtx->logicalDevice, stagingBufferMemory);
        // Creating the Image
        mTextureImage = Utility::CreateImage("Pbr Texture Image", mCtx->physicalDevice, mCtx->logicalDevice, width,
                                             height,
                                             VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_TILING_OPTIMAL,
                                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                             mTextureImageMemory);
        Utility::CreateImageView(mCtx->logicalDevice, mTextureImage, VK_FORMAT_R8G8B8A8_SNORM, mTextureImageView,
                                 VK_IMAGE_ASPECT_COLOR_BIT);
        Utility::TransitionImageLayout(*mCtx, mTextureImage, VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
        Utility::CopyBufferToImage(*mCtx, stagingBuffer, mTextureImage, width, height, VK_IMAGE_ASPECT_COLOR_BIT);
        Utility::TransitionImageLayout(*mCtx, mTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

        // Freeing the staging buffers;
        vkDestroyBuffer(mCtx->logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(mCtx->logicalDevice, stagingBufferMemory, nullptr);

        CreateSampler();
        CreateTextureImageInfo();

    }

    void PbrTexture::CreateSampler() {
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

        vkCreateSampler(mCtx->logicalDevice, &samplerCreateInfo, nullptr, &mTextureImageSampler);
    }

    VkWriteDescriptorSet
    PbrTexture::GetWriteSamplerDescriptorForTexture(VkDescriptorSet descriptorSet, int binding, int elementIndex) {

        VkWriteDescriptorSet writeInfo{};
        writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeInfo.dstArrayElement = elementIndex;
        writeInfo.descriptorCount = 1;
        writeInfo.dstBinding = binding;
        writeInfo.dstSet = descriptorSet;
        writeInfo.pImageInfo = &mTextureImageInfo;
        writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        return writeInfo;

    }

    void PbrTexture::CreateTextureImageInfo() {
        mTextureImageInfo.sampler = mTextureImageSampler;
        mTextureImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mTextureImageInfo.imageView = mTextureImageView;
    }
}