//
// Created by ghima on 31-08-2025.
//
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>

#include "Components/MeshComponent.h"
#include "Entity/GameObject.h"
#include "Components/TextureComponent.h"
#include "Components/TransformComponent.h"
#include "Entity/Scene.h"
#include "Core/ImguiEditor.h"
#include "Core/Logger.h"

namespace vk {
    MeshComponent::MeshComponent(vk::GameObject *gameObject, const std::string &id, List<rn::Vertex> &vertices,
                                 List<std::uint32_t> &indices,
                                 bool calculateNormals,
                                 const std::string &modelTexturePath)
            : Component(gameObject, id), mVertexList{vertices}, mIndexList{indices},
              mCalculateNormals{calculateNormals}, mStaticMesh{nullptr}, mModelTexturePath(modelTexturePath) {
    }

    MeshComponent::~MeshComponent() {
        delete mStaticMesh;
    }

    void MeshComponent::BeginPlay() {
        Component::BeginPlay();
        rn::MATERIAL_TYPE materialType = rn::MATERIAL_TYPE::PHONG;
        std::string textureId = mModelTexturePath.empty() ? rn::BASE_PHONG_MATERIAL_ID : mModelTexturePath;

        std::shared_ptr<TextureComponent> textureComponent = mOwningGameObject->GetComponentType<TextureComponent>();
        if (textureComponent != nullptr) {
            materialType = textureComponent->GetMaterialType();
            textureId = textureComponent->GetTextureId();
        } else {
            LOG_WARN("No Texture Component found in Game Object for {} .. Falling to default Texture", id.c_str());
        }

        mStaticMesh = new rn::StaticMesh{*Component::ctx, mVertexList, mIndexList, mOwningGameObject->GetPickId(),
                                         textureId, mCalculateNormals, materialType};
        // Register the object with the Rendering Context for the Graphics context
        std::shared_ptr<TransformComponent> transformComponent = mOwningGameObject->GetComponentType<TransformComponent>();
        if (transformComponent != nullptr) {
            mStaticMesh->SetModelMatrix(transformComponent->GetModelMatrix());
        }

        Component::ctx->RegisterMesh(id, mStaticMesh);
        ImguiEditor::GetInstance(ctx)->GetGuiViewportDelegate()->Register(this, &MeshComponent::ViewportKeyHandler);
    }

    void MeshComponent::Tick(float DeltaTime) {
        Component::Tick(DeltaTime);
        std::shared_ptr transformComponent = mOwningGameObject->GetComponentType<TransformComponent>();
        if (transformComponent != nullptr) {
            mStaticMesh->SetModelMatrix(transformComponent->GetModelMatrix());
            // Changing the transform matrix based on the gizmo positions;

            bool mousePressed = ImGui::IsMouseDown(ImGuiMouseButton_Left);
            bool mouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

            if (mOwningGameObject->GetPickId() ==
                mOwningGameObject->GetScene()->GetRendererContext()->GetActiveClickedObjectId() &&
                ctx->GetActiveGizmoAxis() != rn::AXIS::NONE) {
                if (ctx->beginGizmoDrag) {
                    gizmoDragController.BeginDrag(static_cast<int>(ctx->GetActiveGizmoAxis()),
                                                  glm::vec3{transformComponent->GetModelMatrix()[3]},
                                                  transformComponent->GetScale(),
                                                  transformComponent->GetRotation(),
                                                  ctx->GetViewProjectionMatrix()->view,
                                                  ctx->GetViewProjectionMatrix()->projection,
                                                  ImguiEditor::GetInstance(ctx)->GetLocalMousePos().x,
                                                  ImguiEditor::GetInstance(ctx)->GetLocalMousePos().y,
                                                  ctx->viewportExtends.width,
                                                  ctx->viewportExtends.height,
                                                  ctx->cameraForward);
                    ctx->beginGizmoDrag = false;
                }
                if (gizmoDragController.IsDragging() && mousePressed) {
                    if (ctx->GetGizmoType() == rn::GIZMO_TYPE::TRANSLATE) {
                        glm::vec3 newPos = gizmoDragController.UpdateDrag(
                                ImguiEditor::GetInstance(ctx)->GetLocalMousePos().x,
                                ImguiEditor::GetInstance(ctx)->GetLocalMousePos().y,
                                ctx->viewportExtends.width,
                                ctx->viewportExtends.height,
                                ctx->GetViewProjectionMatrix()->view,
                                ctx->GetViewProjectionMatrix()->projection
                        );
                        transformComponent->SetPosition(newPos);
                    } else if (ctx->GetGizmoType() == rn::GIZMO_TYPE::SCALE) {
                        glm::vec3 newScale = gizmoDragController.UpdateScale(
                                ImguiEditor::GetInstance(ctx)->GetLocalMousePos().x,
                                ImguiEditor::GetInstance(ctx)->GetLocalMousePos().y,
                                ctx->viewportExtends.width,
                                ctx->viewportExtends.height,
                                ctx->GetViewProjectionMatrix()->view,
                                ctx->GetViewProjectionMatrix()->projection
                        );
                        transformComponent->SetScale(newScale);
                    } else if (ctx->GetGizmoType() == rn::GIZMO_TYPE::ROTATE) {
                        glm::vec3 newRotation = gizmoDragController.UpdateRotation(
                                ImguiEditor::GetInstance(ctx)->GetLocalMousePos().x,
                                ImguiEditor::GetInstance(ctx)->GetLocalMousePos().y,
                                ctx->viewportExtends.width,
                                ctx->viewportExtends.height,
                                ctx->GetViewProjectionMatrix()->view,
                                ctx->GetViewProjectionMatrix()->projection
                        );
                        transformComponent->SetRotation(newRotation);
                    }
                }
                if (mouseReleased) {
                    gizmoDragController.EndDrag();
                    ctx->AddRendererEvent({rn::RendererEvent::Type::MOUSE_RELEASED});
                }
            }
        }
    }

    bool MeshComponent::ViewportKeyHandler() {
        if (ImGui::IsKeyPressed(ImGuiKey_E)) {
            ctx->SetGizmoType(rn::GIZMO_TYPE::TRANSLATE);
        } else if (ImGui::IsKeyPressed(ImGuiKey_R)) {
            ctx->SetGizmoType(rn::GIZMO_TYPE::ROTATE);
        } else if (ImGui::IsKeyPressed(ImGuiKey_T)) {
            ctx->SetGizmoType(rn::GIZMO_TYPE::SCALE);
        }
        return true;
    }
}