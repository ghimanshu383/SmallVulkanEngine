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

    bool TransformComponent::SetUpGuiInspector() {
        if (mOwningGameObject->GetPickId() == Component::ctx->GetActiveClickedObjectId()) {
            float speed = 0.05f;
            float minVal = -100.0f;
            float maxVal = 100.0f;

            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::DragFloat3("POSITION", glm::value_ptr(mModelMatrix[3]), speed, minVal, maxVal, "%.2f",
                                  ImGuiInputTextFlags_EnterReturnsTrue);
            }
        }
        return true;
    }
}