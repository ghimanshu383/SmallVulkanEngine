//
// Created by ghima on 27-08-2025.
//

#include "MainWindow.h"
#include "Utility.h"
#include "Components/MeshComponent.h"
#include "Entity/Scene.h"
#include "Entity/GameObject.h"

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

        mGraphics = new rn::Graphics(mWindow);
        mDefaultScene = new Scene{&mCtx};
        mCtx = mGraphics->GetRendererContext();
        List<rn::Vertex> vertOne = {{glm::vec3{0, -1, 0},   {1, 0, 0, 1}},
                                    {glm::vec3{-0.5, 0, 0}, {0, 1, 0, 1}},
                                    {glm::vec3{0.5, 0, 0},  {0, 0, 1, 1}}};
        List<std::uint32_t> indices = {0, 1, 2};
        std::shared_ptr<GameObject> testObject = mDefaultScene->SpawnGameObject<GameObject>();

        std::string meshName = "Triangle Mesh";
        testObject->SpawnComponent<MeshComponent>(meshName, vertOne, indices);

        mDefaultScene->BeginPlay();
    }

    void MainWindow::RenderWindow() {
        while (!glfwWindowShouldClose(mWindow)) {
            glfwPollEvents();
            mGraphics->BeginFrame();
            mGraphics->Draw();
            mGraphics->EndFrame();
        }
        delete mGraphics;
    }
}