//
// Created by ghima on 27-08-2025.
//

#ifndef SMALLVKENGINE_MAINWINDOW_H
#define SMALLVKENGINE_MAINWINDOW_H
#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"
#include "Graphics.h"
#include "Delegate.h"

namespace vk {
    class Scene;

    class Camera;

    class MainWindow {
    private:
        GLFWwindow *mWindow;
        rn::Graphics *mGraphics = nullptr;
        rn::RendererContext mCtx;
        Scene *mDefaultScene;
        Camera *mDefaultCamera;
        std::shared_ptr<Delegate<>> mRenderLoopDelegate = std::make_shared<Delegate<>>();

        void Init(int width, int height, const char *title);

    public:
        MainWindow(int width, int height, const char *title);

        ~MainWindow();

        void RenderWindow();

        Scene *GetDefaultScene() const { return mDefaultScene; }

        static void KeyBoardInputCallback(GLFWwindow *window, int key, int code, int action, int mode);

        void InitObjects();

        // Getters;
        std::shared_ptr<Delegate<>> GetRenderLoopDelegate() const { return mRenderLoopDelegate; }

        const rn::RendererContext &GetRendererContext() const { return mCtx; }

        // Clean UP
        void DeleteGraphics() {
            if (mGraphics != nullptr) {}
            delete mGraphics;
        }
    };
}
#endif //SMALLVKENGINE_MAINWINDOW_H
