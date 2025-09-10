//
// Created by ghima on 30-08-2025.
//

#ifndef SMALLVKENGINE_GAMEOBJECT_H
#define SMALLVKENGINE_GAMEOBJECT_H

#include "Components/Component.h"
#include "Core/Constants.h"
#include "Utility.h"

namespace vk {
    class Scene;

    class GameObject {
    private:
        List<std::shared_ptr<Component>> mComponentList;
        bool IsPendingDestroy;
        Scene *mScene;
    public:
        explicit GameObject(Scene *scene);

        virtual void BeginPlay();

        virtual void Tick(float deltaTime);

        template<typename T, typename... Args, typename = typename std::enable_if<std::is_base_of<Component, T>::value, T>::type>
        std::shared_ptr<T> SpawnComponent(Args... args) {
            if (GetComponentType<T>() != nullptr) {
                LOG_WARN("Component Already Exists");
                return nullptr;
            }
            std::shared_ptr<T> t = std::make_shared<T>(this, args...);
            mComponentList.push_back(t);
            return t;
        }

        template<typename T>
        std::shared_ptr<T> GetComponentType() {
            for (std::shared_ptr<Component> &comp: mComponentList) {
                if (auto ptr = std::dynamic_pointer_cast<T>(comp)) {
                    return ptr;
                }
            }
            return nullptr;
        }

        Scene *GetScene() const { return mScene; }

        void Destroy() { IsPendingDestroy = true; }

        bool GetIsPendingDestroy() const { return IsPendingDestroy; }
    };
}
#endif //SMALLVKENGINE_GAMEOBJECT_H
