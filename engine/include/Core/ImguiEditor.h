//
// Created by ghima on 23-09-2025.
//

#ifndef SMALLVKENGINE_IMGUIEDITOR_H
#define SMALLVKENGINE_IMGUIEDITOR_H

#include "Utility.h"

namespace vk {
    class ImguiEditor {
    private:
        static rn::RendererContext *mCtx;

        ImguiEditor() = default;

        static ImguiEditor *instance;
    public:
        static ImguiEditor *GetInstance(rn::RendererContext *ctx);

        void RenderGui();
    };
}
#endif //SMALLVKENGINE_IMGUIEDITOR_H
