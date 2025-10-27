#include "views/Menubar.h"

#include <string>

#include "imgui.h"
#include "imgui_internal.h"
#include "View.h"

void Menubar::render() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = ImGui::GetFrameHeight();

    if(ImGui::BeginViewportSideBar("##MainStatusBar", NULL, ImGuiDir_Up, height, window_flags)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Reload"))
                    _reload = true;

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                add_menu_view("Assembly", &_asm_view);
                add_menu_view("Memory", &_memory_view);
                add_menu_view("Registers", &_register_view);
                add_menu_view("Viewport", &_viewport_view);

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::End();
    }
}

bool Menubar::reload() const {
    return _reload;
}

void Menubar::resetReload() {
    _reload = false;
}

void Menubar::add_menu_view(const std::string &name, bool *enabled) const {
    if(ImGui::MenuItem(name.c_str(), nullptr, enabled))
        static_cast<View *>(view)->getViewable(name)->setVisible(*enabled);
}
