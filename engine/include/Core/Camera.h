//
// Created by ghima on 05-09-2025.
//

#ifndef SMALLVKENGINE_CAMERA_H
#define SMALLVKENGINE_CAMERA_H

#include <Utility.h>
#include "glm/glm.hpp"

namespace vk {
    class MainWindow;

    class Camera {
    private:
        MainWindow *mMainWindow = nullptr;
        glm::vec3 mPosition;

        glm::vec3 mFront{0, 0, 0};
        glm::vec3 mWorldUp = glm::vec3{0, 1, 0};
        glm::vec3 mUp{0, 0, 0};
        glm::vec3 mRight{0, 0, 0};
        glm::mat4 mViewMatrix{1};
        glm::mat4 mProjectionMatrix{1};

        float mYaw;
        float mPitch;
        float mMoveSpeed;
        float mTurnSpeed;
        rn::RendererContext *mCtx;

        void Update();

    public:
        Camera(MainWindow *mainWindow, rn::RendererContext *ctx, glm::vec3 position, float moveSpeed, float turnSpeed,
               float startYaw,
               float startPitch);

        void Init();

        bool RegisterEvents();

        void KeyEventListener(bool *Keys);

        const glm::mat4 &GetViewMatrix();

        const glm::mat4 &GetProjectionMatrix();

    };
}
#endif //SMALLVKENGINE_CAMERA_H
