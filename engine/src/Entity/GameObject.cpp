//
// Created by ghima on 30-08-2025.
//
#include "Entity/GameObject.h"
#include "Components/TransformComponent.h"

namespace vk {
    GameObject::GameObject(vk::Scene *scene) : mScene{scene}, IsPendingDestroy{false} {

    }

    void GameObject::BeginPlay() {
        for (const std::shared_ptr<Component> &comp: mComponentList) {
            comp->BeginPlay();
        }
    }

    void GameObject::Tick(float deltaTime) {
        List<std::shared_ptr<Component>>::iterator iter = mComponentList.begin();
        while (iter != mComponentList.end()) {
            if (!(*iter)->GetIsPendingDestroy()) {
                (*iter)->Tick(deltaTime);
                iter++;
            } else {
                iter = mComponentList.erase(iter);
            }
        }
    }
}