//
// Created by ghima on 10-09-2025.
//

#ifndef SMALLVKENGINE_OMNIDIRECTIONALLIGHT_H
#define SMALLVKENGINE_OMNIDIRECTIONALLIGHT_H

#include "Utility.h"

namespace rn {
    class OmniDirectionalLight {
    private:
        RendererContext *mCtx;
        List<VkBuffer> mLightBuffer{};
        List<VkDeviceMemory> mLightBufferMemory{};
        List<VkDescriptorSet> mLightDescriptorSets{};
        struct OmniDirectionalInfo mLightInfo;

        void CreateLightBuffers();

        void CreateLightDescriptorSets();

    public:
        OmniDirectionalLight(const std::string &id, RendererContext *ctx, const OmniDirectionalInfo &info);

        ~OmniDirectionalLight();

        void UpdateLightInfo(OmniDirectionalInfo &info);

        const OmniDirectionalInfo GetOmniDirectionalInfo() const { return mLightInfo; }

        void UpdateLightDescriptorSet(size_t currentImageIndex);

        const VkDescriptorSet
        GetLightDescriptorSets(size_t currentImageIndex) const { return mLightDescriptorSets[currentImageIndex]; };
    };
}
#endif //SMALLVKENGINE_OMNIDIRECTIONALLIGHT_H
