//
// Created by ghima on 23-09-2025.
//
#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"
#include "Core/ImguiEditor.h"

namespace vk {
    ImguiEditor *ImguiEditor::instance = nullptr;
    rn::RendererContext *ImguiEditor::mCtx = nullptr;

    ImguiEditor *ImguiEditor::GetInstance(rn::RendererContext *ctx) {
        if (instance == nullptr) {
            mCtx = ctx;
            instance = new ImguiEditor();
        }
        return instance;
    }

    void ImguiEditor::RenderGui() {
        // Set up the dock window, logger and the inspector window; for new windows the provision will be added in the delegate.
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        ImGui::Begin("Console");
        ImGui::Text("This is a test window");
        ImGui::End();

        ImGui::Begin("Viewport");

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        ImGui::Image((ImTextureID) mCtx->imguiViewPortDescriptors->at(mCtx->currentImageIndex), viewportSize);
        static ImVec2 lastSize{0, 0};
        if (viewportSize.x != lastSize.x || viewportSize.y != lastSize.y) {
            lastSize = viewportSize;
            mCtx->AddRendererEvent({rn::RendererEvent::Type::VIEW_PORT_RESIZE,
                                    static_cast<uint32_t>(viewportSize.x),
                                    static_cast<uint32_t>(viewportSize.y)});
        }
        ImGui::End();

        ImGui::Render();
    }
}