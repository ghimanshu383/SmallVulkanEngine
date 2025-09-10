//
// Created by ghima on 10-09-2025.
//
#include "Components/TextureComponent.h"

namespace vk {
    TextureComponent::TextureComponent(vk::GameObject *ownerGameObject, const std::string &textureId,
                                       rn::RendererContext *ctx) : Component(ownerGameObject, textureId), mCtx{ctx},
                                                                   textureId{textureId}, mTexture{
                    nullptr} {

    }

    void TextureComponent::BeginPlay() {
        Component::BeginPlay();
        mTexture = new rn::Texture(textureId.c_str(), ctx);
        mCtx->RegisterTexture(textureId, mTexture);
    }

    void TextureComponent::Tick(float deltaTime) {
        Component::Tick(deltaTime);
    }
}