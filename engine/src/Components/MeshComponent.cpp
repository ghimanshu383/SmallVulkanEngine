//
// Created by ghima on 31-08-2025.
//
#define GLM_ENABLE_EXPERIMENTAL

#include <utility>
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
                                 List<std::uint32_t> &indices, std::string textureId, bool calculateNormals)
            : Component(gameObject, id), mVertexList{vertices}, mIndexList{indices},
              mTextureId{std::move(textureId)}, mCalculateNormals{calculateNormals} {
    }

    MeshComponent::~MeshComponent() {
        delete mStaticMesh;
    }

    void MeshComponent::BeginPlay() {
        Component::BeginPlay();
        if (mTextureId.empty()) {
            LOG_WARN("No Texture Id provided for {} Checking in the game object for Texture Comp", id.c_str());
            std::shared_ptr<TextureComponent> textureComponent = mOwningGameObject->GetComponentType<TextureComponent>();
            if (textureComponent != nullptr) {
                mTextureId = textureComponent->GetTextureId();
            } else {
                LOG_WARN("No Texture Component found in Game Object for {} .. Falling to default Texture", id.c_str());
            }
        }
        mStaticMesh = new rn::StaticMesh{*Component::ctx, mVertexList, mIndexList, mOwningGameObject->GetPickId(),
                                         mTextureId, mCalculateNormals};
        // Register the object with the Rendering Context for the Graphics context
        std::shared_ptr<TransformComponent> transformComponent = mOwningGameObject->GetComponentType<TransformComponent>();
        if (transformComponent != nullptr) {
            mStaticMesh->SetModelMatrix(transformComponent->GetModelMatrix());
        }

        Component::ctx->RegisterMesh(id, mStaticMesh);
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
                    glm::vec3 newPos = gizmoDragController.UpdateDrag(
                            ImguiEditor::GetInstance(ctx)->GetLocalMousePos().x,
                            ImguiEditor::GetInstance(ctx)->GetLocalMousePos().y,
                            ctx->viewportExtends.width,
                            ctx->viewportExtends.height,
                            ctx->GetViewProjectionMatrix()->view,
                            ctx->GetViewProjectionMatrix()->projection
                    );
                    transformComponent->SetModelMatrixTranslate(newPos);
                }
                if (mouseReleased) {
                    gizmoDragController.EndDrag();
                    ctx->AddRendererEvent({rn::RendererEvent::Type::MOUSE_RELEASED});
                }
            }
        }
    }
}