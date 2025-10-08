//
// Created by ghima on 02-10-2025.
//

#ifndef SMALLVKENGINE_POINTLIGHTS_H
#define SMALLVKENGINE_POINTLIGHTS_H

#include "Utility.h"

namespace rn {
    class PointLights {
    private :
        static std::uint32_t mCurrentLightSizeCount;
        static struct PointLightUBO mPointLightUBO;
        static RendererContext *mCtx;
        List<VkBuffer> mPointLightsBuffer{};
        List<VkDeviceMemory> mPointLightsMemory{};
        List<VkDescriptorSet> mPointLightDescriptorSets{};
        static List<VkDescriptorSet> mPointLightShadowDescriptorSets;
        static List<class PointLightShadowMap *> mPointLightShadowMaps;
        VkSemaphore mPointLightShadowMapSemaphore;
        static VkFence renderShadowSceneFence;
        static List<VkCommandBuffer> mShadowCommandBuffer;
        static List<std::thread> mShadowMapThreads;
        std::mutex mutex_;
        List<VkCommandPool> mThreadedCommandPools{};

        static VkSampler mDummyShadowSampler;
        static VkImageView mDummyShadowImageview;
        VkImage mDummyShadowImage{};
        VkDeviceMemory mDummyImageMemory{};

        void CreatePointLightBuffers();

        void BindPointLightDescriptors();

        static void BindPointLightShadowDescriptors();

        void CreateShadowMapSemaphoreAndAllocateCommandbuffer();

        void CreateDummyShadowBindingContext();

    public:
        explicit PointLights(RendererContext *ctx);

        ~PointLights();


        void UpdatePointLightBuffers(size_t currentImageIndex);

        static std::uint32_t AddPointLight(const PointLightInfo &info);

        static void UpdateLightInfoPosition(const glm::vec4 &position, std::uint32_t lightId);

        void RenderPointLightShadowScene();

        const VkSemaphore &GetShadowMapSemaphore() const { return mPointLightShadowMapSemaphore; }

        const VkDescriptorSet &GetDescriptorSet(size_t currentImageIndex) {
            return mPointLightDescriptorSets[currentImageIndex];
        }

        const VkDescriptorSet &GetShadowDescriptorSet(size_t currentImageIndex) {
            return mPointLightShadowDescriptorSets[currentImageIndex];
        }
    };
}
#endif //SMALLVKENGINE_POINTLIGHTS_H
