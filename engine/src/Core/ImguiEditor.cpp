//
// Created by ghima on 23-09-2025.
//
#include "imgui/imgui.h"
#include "imgui/ImGuizmo.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"
#include "Core/ImguiEditor.h"
#include "Core/Logger.h"

namespace vk {
    ImguiEditor *ImguiEditor::instance = nullptr;
    rn::RendererContext *ImguiEditor::mCtx = nullptr;
    Delegate<> *ImguiEditor::mGuiInspectorDelegate = nullptr;
    Delegate<> *ImguiEditor::mGuiViewportDelegate = nullptr;

    ImguiEditor *ImguiEditor::GetInstance(rn::RendererContext *ctx) {
        if (instance == nullptr) {
            mCtx = ctx;
            mGuiInspectorDelegate = new Delegate<>();
            mGuiViewportDelegate = new Delegate<>();
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


        Logger::GetInstance()->SetUpLogConsole();
        SetupViewport();
        SetupInspectorWindow();
        ImGui::Render();
    }

    void ImguiEditor::SetupViewport() {
        ImGui::Begin("Viewport");
        ImVec2 viewportPos = ImGui::GetCursorScreenPos();
        ImVec2 mousePos = ImGui::GetMousePos();
        localMousePos = {mousePos.x - viewportPos.x, mousePos.y - viewportPos.y};
        mCtx->viewportPos = viewportPos;
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        bool isHovered = ImGui::IsWindowHovered();
        bool isClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left) && isHovered;
        if (isClicked) {
            uint32_t clickX = static_cast<std::uint32_t>(localMousePos.x);
            uint32_t clickY = static_cast<std::uint32_t>(localMousePos.y);
            mCtx->AddRendererEvent({rn::RendererEvent::Type::VIEW_PORT_CLICKED, static_cast<uint32_t>(viewportSize.x),
                                    static_cast<uint32_t>(viewportSize.y),
                                    clickX, clickY});
        }
        ImGui::Image((ImTextureID) mCtx->imguiViewPortDescriptors->at(mCtx->currentImageIndex), viewportSize);
        // Adding the contexts for the gui delegates;
        //mGuiDelegate->Invoke();
        static ImVec2 lastSize{0, 0};
        if (viewportSize.x != lastSize.x || viewportSize.y != lastSize.y) {
            lastSize = viewportSize;
            mCtx->AddRendererEvent({rn::RendererEvent::Type::VIEW_PORT_RESIZE,
                                    static_cast<uint32_t>(viewportSize.x),
                                    static_cast<uint32_t>(viewportSize.y)});
        }
        mGuiViewportDelegate->Invoke();
        ImGui::End();
    }

    void ImguiEditor::SetupInspectorWindow() {
        ImGui::Begin("Inspector");
        if (mCtx->GetActiveClickedObjectId() == 0) {
            ImGui::Text("No Object is Selected");
        }
        mGuiInspectorDelegate->Invoke();
        ImGui::End();
    }
}