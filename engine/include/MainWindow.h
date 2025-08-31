//
// Created by ghima on 27-08-2025.
//

#ifndef SMALLVKENGINE_MAINWINDOW_H
#define SMALLVKENGINE_MAINWINDOW_H
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include "Graphics.h"

namespace vk {
    class Scene;

    class MainWindow {
    private:
        GLFWwindow *mWindow;
        rn::Graphics *mGraphics = nullptr;
        rn::RendererContext mCtx;
        Scene *mDefaultScene;

        void Init(int width, int height, const char *title);

    public:
        MainWindow(int width, int height, const char *title);

        ~MainWindow();

        void RenderWindow();

        Scene *GetDefaultScene() const { return mDefaultScene; }

    };
}
#endif //SMALLVKENGINE_MAINWINDOW_H
