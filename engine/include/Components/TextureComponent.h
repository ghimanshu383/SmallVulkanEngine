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
        rn::RendererContext *mCtx;

    public:
        TextureComponent(GameObject *ownerGameObject, const std::string &textureId,
                         rn::RendererContext *ctx);

        std::string GetTextureId() const { return textureId; };

        virtual void BeginPlay() override;

        virtual void Tick(float deltaTime) override;
    };
}
#endif //SMALLVKENGINE_TEXTURECOMPONENT_H
