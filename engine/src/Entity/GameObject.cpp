//
// Created by ghima on 30-08-2025.
//
#include <Core/ImguiEditor.h>
#include "Entity/GameObject.h"
#include "Components/TransformComponent.h"
#include "Entity/Scene.h"

namespace vk {
    GameObject::GameObject(vk::Scene *scene, std::uint32_t pickId) : mScene{scene}, IsPendingDestroy{false},
                                                                     mPickId{pickId} {

    }

    void GameObject::BeginPlay() {
        for (const std::shared_ptr<Component> &comp: mComponentList) {
            comp->BeginPlay();
        }
        SetUpGameObjectGui();
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

    void GameObject::SetUpGameObjectGui() {
        std::shared_ptr<TransformComponent> transformComponent = GetComponentType<TransformComponent>();
        if (transformComponent) {
            ImguiEditor::GetInstance(mScene->GetRendererContext())->GetGuiInspectorDelegate()->Register(
                    transformComponent.get(), &TransformComponent::SetUpGuiInspector);
        }
    }
}