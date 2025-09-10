//
// Created by ghima on 10-09-2025.
//

#ifndef SMALLVKENGINE_TEXTURE_H
#define SMALLVKENGINE_TEXTURE_H

#include "Utility.h"

namespace rn {
    class Texture {
    private:
        const char *textureId;
        RendererContext *mCtx;
        VkDescriptorSet mTextureDescriptorSet{};
        VkImageView mTextureImageView{};
        VkImage mTextureImage{};
        VkDeviceMemory mTextureImageMemory{};

        void CreateTextureImage(const char *fileName);

        void CreateTexture(const char *fileName);

        void CreateTextureDescriptorSets(VkImageView imageView);

    public:
        Texture(const char *fileName, RendererContext *ctx);

        ~Texture();

        VkDescriptorSet GetTextureDescriptorSet() { return mTextureDescriptorSet; }

    };
}
#endif //SMALLVKENGINE_TEXTURE_H
