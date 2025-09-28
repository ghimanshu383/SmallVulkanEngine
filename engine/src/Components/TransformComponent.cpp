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
        ImVec2 viewportPos = mOwningGameObject->GetScene()->GetRendererContext()->viewportPos;
        ImVec2 viewportSize = {
                static_cast<float>(mOwningGameObject->GetScene()->GetRendererContext()->viewportExtends.width),
                static_cast<float>(mOwningGameObject->GetScene()->GetRendererContext()->viewportExtends.height)};

        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(viewportPos.x, viewportPos.y,
                          viewportSize.x, viewportSize.y);
        static ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
        static ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;

        glm::mat4 projMatrix = mOwningGameObject->GetScene()->GetRendererContext()->GetViewProjectionMatrix()->projection;
        projMatrix[1][1] *= -1;
        float *view = glm::value_ptr(
                mOwningGameObject->GetScene()->GetRendererContext()->GetViewProjectionMatrix()->view);
        float *proj = glm::value_ptr(projMatrix);
        ImGuizmo::SetOrthographic(false);


        // Ensure docking does not block mouse events
        if (ImGuizmo::IsOver() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            ImGui::SetActiveID(0, nullptr);

        ImGuizmo::Manipulate(view, proj,
                             ImGuizmo::TRANSLATE, ImGuizmo::WORLD, glm::value_ptr(mModelMatrix));

        if (ImGuizmo::IsUsing())
            ImGui::Text("Dragging works!");
        return true;
    }
}