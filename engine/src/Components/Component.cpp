//
// Created by ghima on 30-08-2025.
//
#include "Components/Component.h"
#include "Entity/GameObject.h"
#include "Entity/Scene.h"

namespace vk {
    Component::Component(vk::GameObject *gameObject, const std::string &id) : mOwningGameObject{gameObject}, id{id} {
        ctx = mOwningGameObject->GetScene()->GetRendererContext();
    }

    void Component::BeginPlay() {

    }

    void Component::Tick(float deltaTime) {

    }

}