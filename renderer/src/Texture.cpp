//
// Created by ghima on 10-09-2025.
//
#include "Texture.h"

namespace rn {

    Texture::Texture(const char *fileName, RendererContext *ctx) : textureId{fileName}, mCtx{ctx} {
        CreateTexture(fileName);
    }

    Texture::~Texture() {
        vkDestroyImageView(mCtx->logicalDevice, mTextureImageView, nullptr);
        vkDestroyImage(mCtx->logicalDevice, mTextureImage, nullptr);
        vkFreeMemory(mCtx->logicalDevice, mTextureImageMemory, nullptr);

    }

    void Texture::CreateTexture(const char *fileName) {
        CreateTextureImage(fileName);
        Utility::CreateImageView(mCtx->logicalDevice, mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, mTextureImageView,
                                 VK_IMAGE_ASPECT_COLOR_BIT);
        CreateTextureDescriptorSets(mTextureImageView);
    }

    void Texture::CreateTextureImage(const char *fileName) {
        int width, height;
        VkDeviceSize imageSize;
        stbi_uc *imageData = Utility::LoadTextureImage(fileName, width, height, imageSize);
        VkBuffer stagingBuffer{};
        VkDeviceMemory stagingBufferMemory{};
        std::string stagingBufferName = "StagingBufferTexture";
        Utility::CreateBuffer(*mCtx, stagingBuffer, (VK_BUFFER_USAGE_TRANSFER_SRC_BIT), stagingBufferMemory,
                              (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), imageSize,
                              stagingBufferName);
        void *data;
        vkMapMemory(mCtx->logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, imageData, imageSize);
        vkUnmapMemory(mCtx->logicalDevice, stagingBufferMemory);
        stbi_image_free(imageData);


        mTextureImage = Utility::CreateImage("Texture Image", mCtx->physicalDevice, mCtx->logicalDevice, width, height,
                                             VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                             (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                                             (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), mTextureImageMemory);
        Utility::TransitionImageLayout(*mCtx, mTextureImage, VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
        Utility::CopyBufferToImage(*mCtx, stagingBuffer, mTextureImage, width, height, VK_IMAGE_ASPECT_COLOR_BIT);
        Utility::TransitionImageLayout(*mCtx, mTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

        vkDestroyBuffer(mCtx->logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(mCtx->logicalDevice, stagingBufferMemory, nullptr);
    }

    void Texture::CreateTextureDescriptorSets(VkImageView imageView) {
        VkDescriptorSetAllocateInfo allocateInfo{};

        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pSetLayouts = &mCtx->samplerDescriptorSetLayout;
        allocateInfo.descriptorPool = mCtx->samplerDescriptorPool;
        allocateInfo.descriptorSetCount = 1;

        Utility::CheckVulkanError(vkAllocateDescriptorSets(mCtx->logicalDevice, &allocateInfo, &mTextureDescriptorSet),
                                  "Failed to allocate Descriptor set for the texture");

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.sampler = mCtx->textureSampler;
        imageInfo.imageView = imageView;

        VkWriteDescriptorSet writeInfo{};
        writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeInfo.descriptorCount = 1;
        writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeInfo.dstSet = mTextureDescriptorSet;
        writeInfo.dstBinding = 0;
        writeInfo.dstArrayElement = 0;
        writeInfo.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(mCtx->logicalDevice, 1, &writeInfo, 0, nullptr);
    }

}