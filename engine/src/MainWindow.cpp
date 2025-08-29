//
// Created by ghima on 27-08-2025.
//

#include "MainWindow.h"
#include "Utility.h"

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
    }

    void MainWindow::RenderWindow() {
        while (!glfwWindowShouldClose(mWindow)) {
            glfwPollEvents();
        }
        delete mGraphics;
    }
}