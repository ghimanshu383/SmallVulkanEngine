//
// Created by ghima on 05-09-2025.
//
#include "Core/InputSystem.h"

namespace vk {
    InputSystem *InputSystem::instance = nullptr;

    InputSystem::InputSystem() = default;

    InputSystem *InputSystem::GetInstance() {
        if (instance == nullptr) {
            instance = new InputSystem();
        }
        return instance;
    }

    void InputSystem::KeyInputHandler(int key, int code, int action, int mode) {
        if (key < 1024) {
            if (action == GLFW_PRESS) {
                Keys[key] = true;
            } else if (action == GLFW_RELEASE) {
                Keys[key] = false;
            }
        }
    }
}