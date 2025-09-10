//
// Created by ghima on 31-08-2025.
//
#include <utility>

#include "Components/MeshComponent.h"
#include "Entity/GameObject.h"
#include "Components/TextureComponent.h"

namespace vk {
    MeshComponent::MeshComponent(vk::GameObject *gameObject, std::string &id, List<rn::Vertex> &vertices,
                                 List<std::uint32_t> &indices, std::string textureId)
            : Component(gameObject, id), mVertexList{vertices}, mIndexList{indices}, mTextureId{std::move(textureId)} {
        std::shared_ptr<TextureComponent> textureComponent = gameObject->GetComponentType<TextureComponent>();
        if (textureComponent != nullptr) {
            mTextureId = textureComponent->GetTextureId();
        }
        mStaticMesh = new rn::StaticMesh{*Component::ctx, vertices, indices, mTextureId};
    }

    MeshComponent::~MeshComponent() {
        delete mStaticMesh;
    }

    void MeshComponent::BeginPlay() {
        Component::BeginPlay();
        // Register the object with the Rendering Context for the Graphics context

        Component::ctx->RegisterMesh(id, mStaticMesh);
    }

    void MeshComponent::Tick(float DeltaTime) {
        Component::Tick(DeltaTime);
        LOG_INFO("The Tick for the log is called");
    }
}