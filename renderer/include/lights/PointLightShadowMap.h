//
// Created by ghima on 02-10-2025.
//

#ifndef SMALLVKENGINE_POINTLIGHTSHADOWMAP_H
#define SMALLVKENGINE_POINTLIGHTSHADOWMAP_H

#include "Utility.h"

namespace rn {
    class PointLightShadowMap {
    private:
        RendererContext *mCtx;
        PointLightInfo mLightInfo{};
        PointLightViewProjection mViewProjection{};
        LightData mLightData {};
        VkRenderPass mRenderPass{};
        VkPipelineLayout mPipelineLayout{};
        VkPipeline mPipeline{};
        VkImage mShadowImage{};
        VkDeviceMemory mShadowImageMemory{};
        List<VkImageView> mShadowRendingImageViews{};
        List<VkDeviceMemory> mShadowRenderingImageViewsMemory{};
        VkImageView mSamplerImageView{};
        VkSampler mShadowSampler{};
        List<VkFramebuffer> mFrameBuffers{};
        VkViewport mViewPort{};
        VkRect2D mScissors{};
        VkPushConstantRange mModelPushConstant;
        VkCommandBuffer mCommandBuffer{};
        VkSampler mSampler;


        VkBuffer viewProjectionBuffer{};
        VkDeviceMemory viewProjectionMemory{};
        VkDescriptorSet viewProjectionDescriptorSet{};
        VkDescriptorPool viewProjectionDescriptorPool{};
        VkDescriptorSetLayout viewProjectionDescriptorSetLayout{};

        VkBuffer mLightDataBuffer{};
        VkDeviceMemory mLightDataMemory{};
        VkDescriptorSet mLightDataDescriptorSet{};

        void CreateFrameBuffersImagesAndImageViews();

        void CreateRenderPass();

        void CreatePipeline();

        void CreateSampler();

        void CreateDescriptors();

        void CreateCommandBufferAndFences();

        void UpdateDescriptorSet(const ViewProjection &viewProjection);

        void ComputePointLightViewProjection();

    public:
        explicit PointLightShadowMap(RendererContext *ctx, PointLightInfo lightInfo, VkCommandBuffer mCommandbuffer);

        void BeginPointShadowFrame();

        void EndFrame();

        void ImageTransition();

        // Getter;
        const VkSampler &GetSampler() const { return mSampler; };

        const VkImageView &GetCubeImageView() const { return mSamplerImageView; }

        // Setters;
        void UpdateLightInfoInShadowMap(const PointLightInfo &info) {
            mLightInfo = info;
        }
    };
}
#endif //SMALLVKENGINE_POINTLIGHTSHADOWMAP_H
