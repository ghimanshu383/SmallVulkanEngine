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
        RendererContext *mCtx;
        List<VkBuffer> mPointLightsBuffer{};
        List<VkDeviceMemory> mPointLightsMemory{};
        List<VkDescriptorSet> mPointLightDescriptorSets{};

    public:
        explicit PointLights(RendererContext *ctx);

        ~PointLights();

        void CreatePointLightBuffers();

        void BindPointLightDescriptors();

        void UpdatePointLightBuffers(size_t currentImageIndex);

        static std::uint32_t AddPointLight(const PointLightInfo &info);

        static void UpdateLightInfoPosition(const glm::vec4 &position, std::uint32_t lightId);

        const VkDescriptorSet &GetDescriptorSet(size_t currentImageIndex) {
            return mPointLightDescriptorSets[currentImageIndex];
        }
    };
}
#endif //SMALLVKENGINE_POINTLIGHTS_H
