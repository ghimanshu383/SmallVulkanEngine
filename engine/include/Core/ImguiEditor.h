//
// Created by ghima on 23-09-2025.
//

#ifndef SMALLVKENGINE_IMGUIEDITOR_H
#define SMALLVKENGINE_IMGUIEDITOR_H

#include "Utility.h"
#include "Core/Delegate.h"

namespace vk {
    class ImguiEditor {
    private:
        static rn::RendererContext *mCtx;

        ImguiEditor() = default;

        static ImguiEditor *instance;
        static Delegate<> *mGuiInspectorDelegate;
        static Delegate<> *mGuiViewportDelegate;
        ImVec2 localMousePos{};
        bool IsMouseLeftDown{};

        void SetupViewport();

        void SetupInspectorWindow();

    public:
        static ImguiEditor *GetInstance(rn::RendererContext *ctx);

        void RenderGui();

        Delegate<> *GetGuiInspectorDelegate() const { return mGuiInspectorDelegate; };

        Delegate<> *GetGuiViewportDelegate() const { return mGuiViewportDelegate; }


        ImVec2 GetLocalMousePos() {
            return localMousePos;
        }

        bool GetIsMouseLeftDown() const {
            return ImGui::IsMouseDown(ImGuiMouseButton_Left);
        }
    };
}
#endif //SMALLVKENGINE_IMGUIEDITOR_H
