//
// Created by ghima on 11-10-2025.
//
#include "Utility.h"

#ifndef SMALLVKENGINE_PBRMATERIALS_H
#define SMALLVKENGINE_PBRMATERIALS_H
namespace rn {
    class PbrTexture;

    class RendererContext;

    class PbrMaterial {
        RendererContext *mCtx;
        PbrTexture *albedo;
        PbrTexture *normal;
        PbrTexture *metallic;
        PbrTexture *roughness;
        PbrTexture *ao;

        class StaticMesh *mStaticMesh;

        VkDescriptorSetLayout mSetLayout;
        VkDescriptorSet mDescriptorSet{};
        VkDescriptorPool mDescriptorPool{};

        VkBuffer mViewProjectionBuffer{};
        VkDeviceMemory mViewProjectionMemory{};
        VkBuffer mDiffuseLightBuffer{};
        VkDeviceMemory mDiffuseLightMemory{};
        VkBuffer mPointLightBuffer{};
        VkDeviceMemory mPointLightMemory{};

        void CreateDescriptorPoolAndSets();


        void CreateUniformBuffersAndBindToDescriptorSets();


    public:
        explicit PbrMaterial(RendererContext *ctx, VkDescriptorSetLayout setLayout, const std::string &baseTextureLoc,
                             class StaticMesh *mesh);

        void UpdateUniformBuffersFromGraphicsContext();

        StaticMesh *GetStaticMesh() const { return mStaticMesh; }

        const VkDescriptorSet &GetDescriptorSet() const { return mDescriptorSet; }

    };
}
#endif //SMALLVKENGINE_PBRMATERIALS_H
