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
        LightData mLightData{};
        VkRenderPass mRenderPass{};
        VkPipelineLayout mPipelineLayout{};
        VkPipeline mPipeline{};
        VkImage mShadowImage{};
        VkImage mShadowImageDepth;
        VkDeviceMemory mShadowImageDepthMemory{};
        VkImageView mShadowImageDepthView{};
        VkDeviceMemory mShadowImageMemory{};
        List<VkImageView> mShadowRendingImageViews{};
        List<VkDeviceMemory> mShadowRenderingImageViewsMemory{};
        VkImageView mSamplerImageView{};
        VkSampler mShadowSampler{};
        List<VkFramebuffer> mFrameBuffers{};
        VkViewport mViewPort{};
        VkRect2D mScissors{};
        VkPushConstantRange mModelPushConstant;
        VkSampler mSampler;


        VkBuffer viewProjectionBuffer{};
        VkDeviceMemory viewProjectionMemory{};
        VkDescriptorSet viewProjectionDescriptorSet{};
        VkDescriptorPool mDescriptorPool{};
        VkDescriptorSetLayout mDescriptorSetLayout{};

        VkBuffer mLightDataBuffer{};
        VkDeviceMemory mLightDataMemory{};

        void CreateFrameBuffersImagesAndImageViews();

        void CreateRenderPass();

        void CreatePipeline();

        void CreateSampler();

        void CreateDescriptors();

        void CreateCommandBufferAndFences();

        void UpdateDescriptorSet(const ViewProjection &viewProjection);

        void ComputePointLightViewProjection();

    public:
        explicit PointLightShadowMap(RendererContext *ctx, PointLightInfo lightInfo);

        ~PointLightShadowMap();

        void BeginPointShadowFrame(VkCommandBuffer commandBuffer);

        void EndFrame(VkCommandBuffer commandBuffer);

        void ImageTransition(VkCommandBuffer commandBuffer);

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
