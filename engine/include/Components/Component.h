//
// Created by ghima on 30-08-2025.
//

#ifndef SMALLVKENGINE_COMPONENT_H
#define SMALLVKENGINE_COMPONENT_H

#include "Utility.h"

namespace vk {
    class GameObject;

    class Component {
    protected:
        GameObject *mOwningGameObject;
        bool IsPendingDestroy = false;
        rn::RendererContext *ctx;
        std::string id;
    public:
        explicit Component(GameObject *gameObject, const std::string &id);

        virtual void Tick(float deltaTime);

        virtual void BeginPlay();

        void Destroy() { IsPendingDestroy = true; };

        bool GetIsPendingDestroy() const { return IsPendingDestroy; }

        const std::string &GetId() const { return id; }


    };
}
#endif //SMALLVKENGINE_COMPONENT_H
