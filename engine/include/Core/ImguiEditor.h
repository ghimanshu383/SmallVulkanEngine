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
        static Delegate<> *mGuiDelegate;
        ImVec2 localMousePos{};
        bool IsMouseLeftDown{};

    public:
        static ImguiEditor *GetInstance(rn::RendererContext *ctx);

        void RenderGui();

        Delegate<> *GetGuiDelegate() const { return mGuiDelegate; };

        ImVec2 GetLocalMousePos() {
            return localMousePos;
        }

        bool GetIsMouseLeftDown() const {
            return ImGui::IsMouseDown(ImGuiMouseButton_Left);
        }
    };
}
#endif //SMALLVKENGINE_IMGUIEDITOR_H
