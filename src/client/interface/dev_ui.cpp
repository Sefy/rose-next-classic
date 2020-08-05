#include "stdafx.h"

#include "dev_ui.h"

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#include "zz_hash_table.h"
#include "zz_node.h"
#include "zz_system.h"
#include "zz_visible.h"

#ifdef _DEBUG
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);


void
dev_ui_init(HWND handle) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(handle);
    ImGui_ImplDX9_Init(reinterpret_cast<IDirect3DDevice9*>(::getDevice()));
}

void
dev_ui_frame() {
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    zz_system* zz = reinterpret_cast<zz_system*>(::getZnzin());

    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    ImGui::Begin("Developer");

    bool wireframe = ::getUseWireMode();
    bool observer_camera = ::GetObserverCameraOnOff();

    if (ImGui::CollapsingHeader("Client Settings")) {
        ImGui::Checkbox("Disable UI", &g_GameDATA.m_bNoUI);
        ImGui::SameLine();
        ImGui::Checkbox("Filming mode", &g_GameDATA.m_bFilmingMode);
        ImGui::SameLine();
        ImGui::Checkbox("Observer mode", &g_GameDATA.m_bObserverCameraMode);
    }

    if (ImGui::CollapsingHeader("Engine settings")) {
        ImGui::Checkbox("Wireframe", &wireframe);
    }

    if (ImGui::CollapsingHeader("Scene Tree")) {
        if (ImGui::TreeNode("Visibles")) {
            zz_visible* visible;

            zz_hash_table<zz_node*>::iterator it;
            zz_hash_table<zz_node*>* nodes = zz->visibles->get_hash_table();

            for (it = nodes->begin(); it != nodes->end(); it++) {
                visible = (zz_visible*)(*it);
                if (visible->is_visible()) {
                    ImGui::Text(visible->get_name());
                }
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();

    ImGui::EndFrame();

    ::useWireMode(wireframe);
    ::UserObserverCamera(observer_camera);
}

void
dev_ui_render() {
    IDirect3DDevice9* device = reinterpret_cast<IDirect3DDevice9*>(::getDevice());

    DWORD prev_fill_mode = D3DFILL_SOLID;
    device->GetRenderState(D3DRS_FILLMODE, &prev_fill_mode);

    // Never render dev UI as wireframe
    device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    device->SetRenderState(D3DRS_FILLMODE, prev_fill_mode);
}

void
dev_ui_destroy() {
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

bool
dev_ui_proc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!ImGui::GetCurrentContext()) {
        return false;
    }

    if (ImGui_ImplWin32_WndProcHandler(handle, msg, wParam, lParam)) {
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

#else
void
dev_ui_init(HWND handle) {}

void
dev_ui_frame() {}

void
dev_ui_render() {}

void
dev_ui_destroy() {}

bool
dev_ui_proc(HWND handle, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
    return false;
}
#endif