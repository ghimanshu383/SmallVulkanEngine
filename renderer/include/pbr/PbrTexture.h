//
// Created by ghima on 11-10-2025.
//

#ifndef SMALLVKENGINE_PBRTEXTURE_H
#define SMALLVKENGINE_PBRTEXTURE_H
namespace rn {
    class PbrTexture {
    private:
        class RendererContext *mCtx;

        VkImage mTextureImage{};
        VkDeviceMemory mTextureImageMemory{};
        VkImageView mTextureImageView{};
        VkSampler mTextureImageSampler{};
        VkDescriptorImageInfo mTextureImageInfo{};

        void LoadTexture(const std::string &fileName);

        void CreateSampler();

        void CreateTextureImageInfo();

    public:
        explicit PbrTexture(RendererContext *ctx, const std::string &fileName);

        VkWriteDescriptorSet GetWriteSamplerDescriptorForTexture(VkDescriptorSet descriptorSet, int binding, int elementIndex);
    };
}
#endif //SMALLVKENGINE_PBRTEXTURE_H
