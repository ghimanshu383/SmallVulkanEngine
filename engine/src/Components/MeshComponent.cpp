//
// Created by ghima on 31-08-2025.
//
#include <utility>

#include "Components/MeshComponent.h"
#include "Entity/GameObject.h"
#include "Components/TextureComponent.h"
#include "Components/TransformComponent.h"

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
        mStaticMesh = new rn::StaticMesh{*Component::ctx, mVertexList, mIndexList, mTextureId, mCalculateNormals};
        // Register the object with the Rendering Context for the Graphics context

        Component::ctx->RegisterMesh(id, mStaticMesh);
    }

    void MeshComponent::Tick(float DeltaTime) {
        Component::Tick(DeltaTime);
        std::shared_ptr transformComponent = mOwningGameObject->GetComponentType<TransformComponent>();
        if (transformComponent != nullptr) {
            mStaticMesh->SetModelMatrix(transformComponent->GetModelMatrix());
        }
    }
}