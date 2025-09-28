//
// Created by ghima on 13-09-2025.
//

#ifndef SMALLVKENGINE_SKYLIGHT_H
#define SMALLVKENGINE_SKYLIGHT_H

#include "GameObject.h"
#include "lights/OmniDirectionalLight.h"
#include "Utility.h"

namespace vk {
    class SkyLight : public GameObject {
    private:
        rn::OmniDirectionalLight *mDirectionalLight{nullptr};
        std::string m_id;
        rn::OmniDirectionalInfo mDirectionalLightInfo;
    public:
        explicit SkyLight(class Scene *, std::uint32_t pickId, std::string id, const rn::OmniDirectionalInfo &lightInfo);

        virtual void BeginPlay() override;

        virtual void Tick(float DeltaTime) override;

        // Getters
        const rn::OmniDirectionalInfo &GetLightInfo() const { return mDirectionalLightInfo; };

        rn::OmniDirectionalLight *GetDirectionalLight() const { return mDirectionalLight; };
    };
}
#endif //SMALLVKENGINE_SKYLIGHT_H
