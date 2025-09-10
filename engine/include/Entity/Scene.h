//
// Created by ghima on 31-08-2025.
//

#ifndef SMALLVKENGINE_SCENE_H
#define SMALLVKENGINE_SCENE_H

#include "Core/Constants.h"
#include "Utility.h"
#include "GameObject.h"

namespace vk {
    class Scene {
    private:
        rn::RendererContext *mCtx;
        List<std::shared_ptr<GameObject>> mGameObjects;
    public:
        explicit Scene(rn::RendererContext *ctx);

        void BeginPlay();

        void Tick(float deltaTime);

        template<typename T, typename... Args, typename = typename std::enable_if<std::is_base_of<GameObject, T>::value>::type>
        std::shared_ptr<T> SpawnGameObject(Args... args) {
            std::shared_ptr<T> gameObject = std::make_shared<T>(this, args...);
            mGameObjects.push_back(gameObject);
            return gameObject;
        }

        rn::RendererContext *GetRendererContext() const { return mCtx; }
    };
}
#endif //SMALLVKENGINE_SCENE_H
