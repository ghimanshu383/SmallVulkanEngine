//
// Created by ghima on 05-09-2025.
//
#include "Core/Camera.h"
#include "Utility.h"
#include "Core/MainWindow.h"
#include "Core/InputSystem.h"

namespace vk {
    Camera::Camera(MainWindow *mainWindow, rn::RendererContext *ctx, glm::vec3 position, float moveSpeed,
                   float turnSpeed, float startYaw,
                   float startPitch) :
            mMainWindow{mainWindow}, mCtx{ctx}, mPosition(position), mMoveSpeed{moveSpeed}, mTurnSpeed{turnSpeed},
            mPitch{startPitch}, mYaw{startYaw} {

    }

    void Camera::Init() {
        mProjectionMatrix = glm::perspective(glm::radians(45.f),
                                             (float) mCtx->windowExtents.width / (float) mCtx->windowExtents.height,
                                             0.1f, 100.f);
        mProjectionMatrix[1][1] *= -1;
        Update();
        mMainWindow->GetRenderLoopDelegate()->Register<Camera>(this, &Camera::RegisterEvents);

    }

    bool Camera::RegisterEvents() {

        mProjectionMatrix = glm::perspective(glm::radians(45.f),
                                             (float) mCtx->viewportExtends.width / (float) mCtx->viewportExtends.height,
                                             0.1f, 100.f);
        mProjectionMatrix[1][1] *= -1;

        KeyEventListener(InputSystem::GetInstance()->GetKeys());

        mMainWindow->GetRendererContext()->UpdateViewAndProjectionMatrix({mProjectionMatrix, mViewMatrix});
        return true;
    }

    void Camera::KeyEventListener(bool *keys) {
        if (keys[GLFW_KEY_S]) {
            mPosition -= mFront * mMoveSpeed;
        }
        if (keys[GLFW_KEY_W]) {
            mPosition += mFront * mMoveSpeed;
        }
        if (keys[GLFW_KEY_A]) {
            mPosition -= mRight * mMoveSpeed;
        }
        if (keys[GLFW_KEY_D]) {
            mPosition += mRight * mMoveSpeed;
        }
        Update();
    }

    const glm::mat4 &Camera::GetViewMatrix() {
        return mViewMatrix;
    }

    const glm::mat4 &Camera::GetProjectionMatrix() {
        return mProjectionMatrix;
    }

    void Camera::Update() {
        mFront.x = glm::cos(glm::radians(mPitch)) * glm::cos(glm::radians(mYaw));
        mFront.y = glm::sin(glm::radians(mPitch));
        mFront.z = glm::sin(glm::radians(mYaw) * glm::cos(glm::radians(mPitch)));
        mFront = glm::normalize(mFront);

        mRight = glm::normalize(glm::cross(mFront, mWorldUp));
        mUp = glm::normalize(glm::cross(mRight, mFront));

        mViewMatrix = glm::lookAt(mPosition, mPosition + mFront, mWorldUp);
    }
}