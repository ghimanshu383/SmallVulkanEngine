//
// Created by ghima on 02-10-2025.
//
#include "Entity/GameObject.h"

#ifndef SMALLVKENGINE_POINTLIGHT_H
#define SMALLVKENGINE_POINTLIGHT_H
namespace vk {
    class Scene;

    class PointLight : public GameObject {
    private:
        std::string mStringId;
        rn::RendererContext *mCtx;
        rn::PointLightInfo mLightInfo{};
        std::uint32_t mLightId = -1;
    public:
        PointLight(Scene *scene, std::uint32_t pickId, const std::string &stringId,
                   const rn::PointLightInfo &mLightInfo);

        virtual void BeginPlay() override;

        virtual void Tick(float delta) override;
    };
}
#endif //SMALLVKENGINE_POINTLIGHT_H
