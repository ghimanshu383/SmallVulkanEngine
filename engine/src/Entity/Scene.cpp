//
// Created by ghima on 31-08-2025.
//
#include "Entity/Scene.h"

namespace vk {
    Scene::Scene(rn::RendererContext *ctx) : mCtx{ctx} {

    }

    void Scene::BeginPlay() {
        for (std::shared_ptr<GameObject> &gameObject: mGameObjects) {
            gameObject->BeginPlay();
        }
    }

    void Scene::Tick(float deltaTime) {
        List<std::shared_ptr<GameObject>>::iterator iter = mGameObjects.begin();
        while (iter != mGameObjects.end()) {
            if (!(*iter)->GetIsPendingDestroy()) {
                (*iter)->Tick(deltaTime);
                iter++;
            } else {
                iter = mGameObjects.erase(iter);
            }
        }
    }
}