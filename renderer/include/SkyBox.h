//
// Created by ghima on 06-10-2025.
//
#include "Utility.h"

#ifndef SMALLVKENGINE_SKYBOX_H
#define SMALLVKENGINE_SKYBOX_H
namespace rn {
    class Skybox {
    private:
        VkPipeline mPipeline{};
        RendererContext *mCtx;
        VkPipelineLayout mLayout{};
        VkDescriptorSetLayout mSetLayout{};
        VkDescriptorPool mDescriptorPool{};
        VkDescriptorSet mDescriptorSets{};
        VkViewport mViewport{};
        VkRect2D mScissors{};
        VkImage mSkyBoxImage{};
        VkImageView mSkyBoxImageView{};
        VkDeviceMemory mSkyBoxImageMemory{};
        List<VkBuffer> mStagingBuffers{};
        List<VkDeviceMemory> mStagingBufferMemory{};
        VkSampler mCubeSampler{};

        class StaticMesh *mCubeMesh;

        void CreatePipeline();

        void SimpleCubeMeshBox();

        void CreateDescriptorPoolAndAllocateSets();

        void CreateImageAndImageViews();

        void CreateSamplerAndWriteDescriptorSet();

    public:
        explicit Skybox(RendererContext *ctx);

        void RenderSkyBox();

    };
}
#endif //SMALLVKENGINE_SKYBOX_H
