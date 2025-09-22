//
// Created by ghima on 23-09-2025.
//
#include "Core/ImguiEditor.h"

namespace vk {
    ImguiEditor *ImguiEditor::instance = nullptr;

    ImguiEditor *ImguiEditor::GetInstance() {
        if (instance == nullptr) {
            instance = new ImguiEditor();
        }
        return instance;
    }

    void ImguiEditor::Init() {
        // Set up the dock window, logger and the inspector window; for new windows the provision will be added in the delegate.
    }

    void ImguiEditor::Tick(float DeltaTime) {

    }
}