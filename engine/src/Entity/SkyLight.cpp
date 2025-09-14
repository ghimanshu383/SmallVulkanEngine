//
// Created by ghima on 13-09-2025.
//
#include <utility>

#include "Entity/SkyLight.h"
#include "Entity/Scene.h"

namespace vk {
    SkyLight::SkyLight(Scene *scene, std::string id, const rn::OmniDirectionalInfo &directionalInfo) : GameObject(
            scene), m_id(std::move(id)), mDirectionalLightInfo{directionalInfo} {

    }

    void SkyLight::BeginPlay() {
        mDirectionalLight = new rn::OmniDirectionalLight(m_id, GameObject::GetScene()->GetRendererContext(),
                                                         mDirectionalLightInfo);
        GameObject::GetScene()->GetRendererContext()->SetUpAsDirectionalLight(mDirectionalLight);
        GameObject::BeginPlay();
    }

    void SkyLight::Tick(float DeltaTime) {
        GameObject::Tick(DeltaTime);
    }
}