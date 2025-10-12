//
// Created by ghima on 10-09-2025.
//

#ifndef SMALLVKENGINE_TEXTURECOMPONENT_H
#define SMALLVKENGINE_TEXTURECOMPONENT_H

#include "Component.h"
#include "Texture.h"

namespace vk {
    class TextureComponent : public Component {
    protected:
        std::string textureId;
        rn::Texture *mTexture;
        rn::PbrMaterial *mMaterial;
        rn::RendererContext *mCtx;
        rn::MATERIAL_TYPE mMaterialType;

    public:
        TextureComponent(GameObject *ownerGameObject, const std::string &textureId,
                         rn::RendererContext *ctx, rn::MATERIAL_TYPE materialType);

        std::string GetTextureId() const { return textureId; };

        virtual void BeginPlay() override;

        virtual void Tick(float deltaTime) override;

        rn::MATERIAL_TYPE GetMaterialType() const { return mMaterialType; }
    };
}
#endif //SMALLVKENGINE_TEXTURECOMPONENT_H
