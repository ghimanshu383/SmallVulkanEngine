//
// Created by ghima on 13-09-2025.
//
#define GLM_ENABLE_EXPERIMENTAL

#include <imgui/imgui_internal.h>
#include <glm/gtx/euler_angles.hpp>
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
        mPosition = value;
        UpdateModelMatrix();
    }

    void TransformComponent::setRotation(glm::vec3 rotations) {
        mRotations = rotations;
        UpdateModelMatrix();
    }

    void TransformComponent::setScale(glm::vec3 value) {
        mScale = value;
        UpdateModelMatrix();
    }

    void TransformComponent::UpdateModelMatrix() {
        mModelMatrix = glm::mat4(1.0f);
        mModelMatrix = glm::translate(mModelMatrix, mPosition);
        mModelMatrix *= glm::yawPitchRoll(glm::radians(mRotations.y), glm::radians(mRotations.x),
                                          glm::radians(mRotations.z));
        mModelMatrix = glm::scale(mModelMatrix, mScale);
    }

    bool TransformComponent::SetUpGuiInspector() {
        if (mOwningGameObject->GetPickId() == Component::ctx->GetActiveClickedObjectId()) {
            float speed = 0.05f;
            float minVal = -100.0f;
            float maxVal = 100.0f;

            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::DragFloat3("POSITION", glm::value_ptr(mPosition), speed, minVal, maxVal, "%.2f",
                                      ImGuiInputTextFlags_EnterReturnsTrue)) {
                    UpdateModelMatrix();
                }
                if (ImGui::DragFloat3("ROTATION", glm::value_ptr(mRotations), speed, -180, 180, "%.2f")) {
                    UpdateModelMatrix();
                }
                if (ImGui::DragFloat3("SCALE", glm::value_ptr(mScale), speed, minVal, maxVal, "%.2f")) {
                    UpdateModelMatrix();
                }
            }
        }
        return true;
    }


}