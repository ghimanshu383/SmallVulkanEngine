//
// Created by ghima on 13-09-2025.
//
#define GLM_ENABLE_EXPERIMENTAL

#include <utility>
#include <Components/TransformComponent.h>
#include <glm/gtx/string_cast.hpp>
#include "Entity/SkyLight.h"
#include "Entity/Scene.h"

namespace vk {
    SkyLight::SkyLight(Scene *scene, std::uint32_t pickId, std::string id,
                       const rn::OmniDirectionalInfo &directionalInfo) : GameObject(
            scene, pickId), m_id(std::move(id)), mDirectionalLightInfo{directionalInfo} {

    }

    void SkyLight::BeginPlay() {
        mDirectionalLight = new rn::OmniDirectionalLight(m_id, GameObject::GetScene()->GetRendererContext(),
                                                         mDirectionalLightInfo);
        GameObject::GetScene()->GetRendererContext()->SetUpAsDirectionalLight(mDirectionalLight);
        GameObject::BeginPlay();
    }

    void SkyLight::Tick(float DeltaTime) {
        GameObject::Tick(DeltaTime);
        // Setting up the Direction of the light based on the mesh;
        std::shared_ptr<TransformComponent> transformComponent = GetComponentType<TransformComponent>();
        if (transformComponent != nullptr) {
            mDirectionalLight->SetLightPosition(transformComponent->GetModelMatrix()[3]);
        }
    }
}