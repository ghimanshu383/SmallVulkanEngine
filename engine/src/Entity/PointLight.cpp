//
// Created by ghima on 02-10-2025.
//
#include <Components/TransformComponent.h>
#include <Components/MeshComponent.h>
#include <Components/TextureComponent.h>
#include "Entity/PointLight.h"
#include "Entity/Scene.h"
#include "Core/Logger.h"

namespace vk {
    PointLight::PointLight(vk::Scene *scene, std::uint32_t pickId, const std::string &stringId,
                           const rn::PointLightInfo &lightInfo) : mCtx{
            scene->GetRendererContext()}, GameObject(scene, pickId, stringId), mLightInfo(lightInfo) {

    }

    void PointLight::BeginPlay() {
        mLightId = mCtx->AddPointLight(mLightInfo);
        if (mLightId == -1) {
            Logger::GetInstance()->WriteLog({LogType::ERROR, "Max Point Lights Reached Cant add more"});
            return;
        }
        // Setting up the point light source for mesh and transform component.
        std::shared_ptr<TransformComponent> transformComponent = GameObject::GetComponentType<TransformComponent>();
        if (transformComponent == nullptr) {
            transformComponent = GameObject::SpawnComponent<TransformComponent>("Point Light Transform Component");
        }
        transformComponent->SetPosition(mLightInfo.position);
        transformComponent->SetScale({.2, .2, .2});
        std::shared_ptr<MeshComponent> meshComponent = GameObject::GetComponentType<MeshComponent>();
        if (meshComponent == nullptr) {
            // Setting the default mesh for the point light;
            List<rn::Vertex> cubeVertices = {
                    // Front face (Z+)
                    {{-0.5f, -0.5f, 0.5f},  {1, 0, 0, 1}, {0, 0}},
                    {{0.5f,  -0.5f, 0.5f},  {0, 1, 0, 1}, {1, 0}},
                    {{0.5f,  0.5f,  0.5f},  {0, 0, 1, 1}, {1, 1}},
                    {{-0.5f, 0.5f,  0.5f},  {1, 1, 0, 1}, {0, 1}},

                    // Back face (Z-)
                    {{-0.5f, -0.5f, -0.5f}, {1, 0, 1, 1}, {1, 0}},
                    {{0.5f,  -0.5f, -0.5f}, {0, 1, 1, 1}, {0, 0}},
                    {{0.5f,  0.5f,  -0.5f}, {1, 1, 1, 1}, {0, 1}},
                    {{-0.5f, 0.5f,  -0.5f}, {0, 0, 0, 1}, {1, 1}},
            };

            List<std::uint32_t> cubeIndices = {
                    // Front face
                    0, 1, 2, 0, 2, 3,
                    // Back face
                    4, 6, 5, 4, 7, 6,
                    // Left face
                    4, 3, 7, 4, 0, 3,
                    // Right face
                    1, 5, 6, 1, 6, 2,
                    // Top face
                    3, 2, 6, 3, 6, 7,
                    // Bottom face
                    4, 5, 1, 4, 1, 0
            };
            SpawnComponent<TextureComponent>(R"(D:\cProjects\SmallVkEngine\textures\default.jpg)", mCtx);
            SpawnComponent<MeshComponent>("Point Light Mesh", cubeVertices, cubeIndices, "", true);
        }
        GameObject::BeginPlay();
    }

    void PointLight::Tick(float delta) {
        std::shared_ptr<TransformComponent> transformComponent = GetComponentType<TransformComponent>();
        mLightInfo.position = glm::vec4(transformComponent->GetPosition(), 1.0);
        mCtx->UpdateLightInfoPosition(mLightInfo.position, mLightId);
        GameObject::Tick(delta);
    }
}