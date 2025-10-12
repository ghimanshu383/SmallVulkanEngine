//
// Created by ghima on 10-09-2025.
//
#include "Components/TextureComponent.h"

namespace vk {
    TextureComponent::TextureComponent(vk::GameObject *ownerGameObject, const std::string &textureId,
                                       rn::RendererContext *ctx, rn::MATERIAL_TYPE materialType) : Component(
            ownerGameObject, textureId), mCtx{ctx},
                                                                                                   textureId{textureId},
                                                                                                   mTexture{nullptr},
                                                                                                   mMaterial{nullptr},
                                                                                                   mMaterialType{
                                                                                                           materialType} {

    }

    void TextureComponent::BeginPlay() {
        Component::BeginPlay();
        if (mMaterialType == rn::MATERIAL_TYPE::PHONG) {
            mTexture = mCtx->RegisterTexture(textureId);
        } else {
            mMaterial = mCtx->RegisterPbrMaterial(textureId);
        }
    }

    void TextureComponent::Tick(float deltaTime) {
        Component::Tick(deltaTime);
    }
}