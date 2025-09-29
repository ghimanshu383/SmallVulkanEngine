//
// Created by ghima on 13-09-2025.
//
#define GLM_ENABLE_EXPERIMENTAL

#include <imgui/imgui_internal.h>
#include "Components/TransformComponent.h"
#include "Entity/GameObject.h"
#include "imgui/ImGuizmo.h"
#include "Entity/Scene.h"
#include "Core/ImguiEditor.h"
#include "glm/gtx/string_cast.hpp"

namespace vk {
    TransformComponent::TransformComponent(vk::GameObject *gameObject, const std::string &id) : Component(gameObject,
                                                                                                          id) {

    }

    void TransformComponent::BeginPlay() {
        Component::BeginPlay();
        ImguiEditor::GetInstance(mOwningGameObject->GetScene()->GetRendererContext())->GetGuiDelegate()->Register(this,
                                                                                                                  &TransformComponent::SetUpTransformGizmo);
    }

    void TransformComponent::setTranslate(glm::vec3 value) {
        mModelMatrix = glm::translate(mModelMatrix, value);
    }

    void TransformComponent::setRotationX(float value) {
        mModelMatrix = glm::rotate(mModelMatrix, glm::radians(value), glm::vec3{1, 0, 0});
    }

    void TransformComponent::setRotationY(float value) {
        mModelMatrix = glm::rotate(mModelMatrix, glm::radians(value), glm::vec3{0, 1, 0});
    }

    void TransformComponent::setRotationZ(float value) {
        mModelMatrix = glm::rotate(mModelMatrix, glm::radians(value), glm::vec3{0, 0, 1});
    }

    void TransformComponent::setScale(glm::vec3 value) {
        mModelMatrix = glm::scale(mModelMatrix, value);
    }

    bool TransformComponent::SetUpTransformGizmo() {

        return true;
    }
}