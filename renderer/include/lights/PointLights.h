//
// Created by ghima on 02-10-2025.
//

#ifndef SMALLVKENGINE_POINTLIGHTS_H
#define SMALLVKENGINE_POINTLIGHTS_H

#include "Utility.h"

namespace rn {
    class PointLights {
    private :
        std::uint32_t mCurrentLightSizeCount = 0;
        struct PointLightUBO mPointLightUBO{};
        RendererContext *mCtx;
        List<VkBuffer> mPointLightsBuffer{};
        List<VkDeviceMemory> mPointLightsMemory{};
        List<VkDescriptorSet> mPointLightDescriptorSets{};
        List<class StaticMesh *> mMeshList{};
    public:
        explicit PointLights(RendererContext *ctx);

        ~PointLights();

        void CreatePointLightBuffers();

        void BindPointLightDescriptors();

        void UpdatePointLightBuffers(size_t currentImageIndex);

        void AddPointLight(class StaticMesh *mesh);

        const VkDescriptorSet &GetDescriptorSet(size_t currentImageIndex) {
            return mPointLightDescriptorSets[currentImageIndex];
        }
    };
}
#endif //SMALLVKENGINE_POINTLIGHTS_H
