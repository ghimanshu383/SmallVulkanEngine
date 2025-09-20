//
// Created by ghima on 27-08-2025.
//

#include "Core/MainWindow.h"
#include "Utility.h"
#include "Components/MeshComponent.h"
#include "Entity/Scene.h"
#include "Entity/GameObject.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include "Core/InputSystem.h"
#include "Core/Camera.h"
#include "Components/TextureComponent.h"
#include "Entity/SkyLight.h"
#include "Components/TransformComponent.h"
#include "Components/ModelComponent.h"

namespace vk {
    MainWindow::MainWindow(int width, int height, const char *title) {
        Init(width, height, title);
    }

    MainWindow::~MainWindow() {
        if (mWindow != nullptr) {
            glfwDestroyWindow(mWindow);
        }
        glfwTerminate();
    }

    void MainWindow::Init(int width, int height, const char *title) {
        if (!glfwInit()) {
            LOG_ERROR("Failed to initialize Glfw window");
            std::exit(EXIT_FAILURE);
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!mWindow) {
            LOG_ERROR("Failed to create the glfw window");
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }
        glfwSetWindowUserPointer(mWindow, this);
        glfwSetKeyCallback(mWindow, &MainWindow::KeyBoardInputCallback);

        InitObjects();

    }

    void MainWindow::RenderWindow() {
        while (!glfwWindowShouldClose(mWindow)) {
            glfwPollEvents();
            mRenderLoopDelegate->Invoke();
            // Implement the delta Time functionality later;
            mDefaultScene->Tick(1.f);

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("New Window");
            ImGui::Text("This is a test window");
            ImGui::End();
            ImGui::Render();
            mGraphics->BeginFrame();
            mGraphics->Draw();
            mGraphics->EndFrame();
//s
        }
        delete mGraphics;
    }

    void MainWindow::KeyBoardInputCallback(GLFWwindow *window, int key, int code, int action, int mode) {
        MainWindow *thisWindow = reinterpret_cast<MainWindow *>(glfwGetWindowUserPointer(window));
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
            thisWindow->DeleteGraphics();
        }
        InputSystem::GetInstance()->KeyInputHandler(key, code, action, mode);
    }

    void MainWindow::InitObjects() {
        mGraphics = new rn::Graphics(mWindow);
        mCtx = mGraphics->GetRendererContext();

        mDefaultScene = new Scene{&mCtx};
        mDefaultCamera = new Camera{this, glm::vec3{0, 1, -6}, .005f, .1f, 90.f, 0.f};
        mDefaultCamera->Init();

        List<rn::Vertex> vertOne = {{glm::vec3{-0.5, -1, 0}, {1, 0, 0, 1}, {0, 1}},
                                    {glm::vec3{-0.5, 0, 0},  {0, 1, 0, 1}, {0, 0}},
                                    {glm::vec3{0.5, 0, 0},   {0, 0, 1, 1}, {1, 0}},
                                    {glm::vec3{0.5, -1, 0},  {1, 0, 0, 1}, {1, 1}}};
        List<std::uint32_t> indices = {0, 1, 2,
                                       0, 2, 3};

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
//        std::shared_ptr<GameObject> testObject = mDefaultScene->SpawnGameObject<GameObject>();
//
//        std::string meshName = "Triangle Mesh";
//
//        testObject->SpawnComponent<TextureComponent>(R"(D:\cProjects\SmallVkEngine\textures\brick.png)", &mCtx);
//        // testObject->SpawnComponent<MeshComponent>(meshName, vertOne, indices, "", true);
//        std::string tranName = meshName + " Transform";
//        std::shared_ptr tranComp = testObject->SpawnComponent<TransformComponent>(meshName);
//        testObject->SpawnComponent<ModelComponent>("Test Comp",
//                                                   R"(D:\cProjects\SmallVkEngine\models\simpleModels\building-a.obj)");
//        tranComp->setScale({2, 2, 2});


        std::shared_ptr<GameObject> plane = mDefaultScene->SpawnGameObject<GameObject>();
        plane->SpawnComponent<MeshComponent>("Plane", vertOne, indices, "", true);
        std::shared_ptr<TransformComponent> planTran = plane->SpawnComponent<TransformComponent>(
                "PlaneTransformComponent");
        planTran->setTranslate({0, 0, 0});
        planTran->setRotationX(70.f);
        planTran->setScale({4, 4, 4,});


        std::shared_ptr<GameObject> objectTwo = mDefaultScene->SpawnGameObject<GameObject>();
        objectTwo->SpawnComponent<MeshComponent>("CubeMesh", cubeVertices, cubeIndices, "", true);
        std::shared_ptr<TransformComponent> objectTwoTran = objectTwo->SpawnComponent<TransformComponent>(
                "ObjectTwoTranComponent");
        objectTwoTran->setTranslate({0, .25, -2});
        objectTwoTran->setScale({.5, 0.5, .5});
        // Setting up the default Directional Light;
        rn::OmniDirectionalInfo skyLightInfo{};
        skyLightInfo.position = {0, 3, -7, 1};
        skyLightInfo.color = {1, 1, 1, 1};
        skyLightInfo.intensities = {.7, 1, 0, 0};


        std::shared_ptr<SkyLight> skyLight = mDefaultScene->SpawnGameObject<SkyLight>("Default Sky Light",
                                                                                      skyLightInfo);

        skyLight->SpawnComponent<TextureComponent>(R"(D:\cProjects\SmallVkEngine\textures\default.jpg)", &mCtx);
        //  skyLight->SpawnComponent<MeshComponent>("Sky Light Mesh", cubeVertices, cubeIndices, "", true);
        std::shared_ptr<TransformComponent> transformComponent = skyLight->SpawnComponent<TransformComponent>(
                "Sky Light transform Component");
        transformComponent->setTranslate(skyLight->GetLightInfo().position);
        transformComponent->setScale({.3, .3, .3});
        mDefaultScene->BeginPlay();

        testMap = skyLight->GetDirectionalLight()->GetShadowMap();

    }
}