//
// Created by ghima on 14-09-2025.
//

#ifndef SMALLVKENGINE_SHADOWMAP_H
#define SMALLVKENGINE_SHADOWMAP_H

#include "Utility.h"

namespace rn {
    class ShadowMap {
    private:
        RendererContext *mCtx;

        class OmniDirectionalLight *mDirectionalLight;

        int mWidth;
        int mHeight;

        VkImage mSceneImage{};
        VkImageView mSceneImageview{};
        VkDeviceMemory mSceneImageMemory{};
        VkFramebuffer mShadowFrameBuffer{};
        List<VkFramebuffer> mShadowDebugFrameBuffers{};
        VkBuffer mViewProjectionBuffer{};
        VkDeviceMemory mViewProjectionMemory{};
        VkSampler mShadowSampler{};
        VkRenderPass mShadowRenderPass{};
        VkPipelineLayout mShadowPipelineLayout{};
        VkPipeline mShadowPipeline{};
        VkViewport mViewPort{};
        VkRect2D mScissors{};

        VkCommandBuffer mShadowCommandBuffer{};
        VkSemaphore mShadowMapSemaphore{};
        VkFence mPresentationFinishFence{};
        VkSemaphore mGetNextImageSemaphore{};
        uint32_t mCurrentImageIndex;

        VkPushConstantRange mModelPushConstant{};
        VkDescriptorSetLayout mShadowDescriptorLayout{};
        VkDescriptorPool mShadowDescriptorPool{};
        VkDescriptorSet mShadowDescriptorSet{};

        // Debug Image;
        VkBuffer mDebugBuffer{};
        VkDeviceMemory mDebugBufferMemory{};

        Map<std::string, class StaticMesh *, std::hash<std::string>> *mObjectMap;
    public:
        ShadowMap(RendererContext *ctx, OmniDirectionalLight *light, int width, int height,
                  Map<std::string, StaticMesh *, std::hash<std::string>> *objectMap);

        ~ShadowMap();

        void Init();

        void CreateRenderPass();

        void CreateFrameBuffers();

        void CreatePipeline();

        void CreateDescriptorSetLayout();

        void CreateDescriptorSet();

        void BeginShadowFrame();

        void EndShadowFrame();

        void CreateCommandBuffer();

        void CreateShadowMapSemaphore();

        void WriteViewProjectionDescriptor();

        void UpdateViewProjectionMatrix(const ViewProjection &viewProjection);

        void CreateSampler();

        void ReCreateResourcesForWindowResize();

        const VkSemaphore &GetShadowMapSemaphore() const { return mShadowMapSemaphore; };

        void CreateDebugTransitions();

        void WriteDebugBufferToImage();

        void CreateDebugDisplayFrameBuffers();


    };
}
#endif //SMALLVKENGINE_SHADOWMAP_H
