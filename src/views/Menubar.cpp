#include "views/Menubar.h"

#include <string>

#include "Config.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Util.h"
#include "View.h"

Menubar::Menubar(Naratte *naratte) : _naratte(naratte) {
}

void Menubar::render() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = ImGui::GetFrameHeight();

    if(ImGui::BeginViewportSideBar("##MainStatusBar", NULL, ImGuiDir_Up, height, window_flags)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Settings"))
                    static_cast<View *>(view)->getViewable("Settings")->setVisible(true);
                if (ImGui::MenuItem("Export"))
                    static_cast<View *>(view)->getViewable("Export")->setVisible(true);
                if (ImGui::MenuItem("Import")) {
                    const std::string path = Util::OpenExplorer("*", "Naratte Import Dump");
                    if (!path.empty())
                        _naratte->deserialize(path);
                }
                if (ImGui::MenuItem("Cartridge Info"))
                    static_cast<View *>(view)->getViewable("Cartridge Info")->setVisible(true);
                if (ImGui::MenuItem("Exit"))
                    exit(1);

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                add_menu_view("Assembly", &_asm_view);
                add_menu_view("Memory", &_memory_view);
                add_menu_view("Registers", &_register_view);
                add_menu_view("Viewport", &_viewport_view);

                ImGui::EndMenu();
            }

            ImGui::Dummy(ImVec2(1, 0));
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::Dummy(ImVec2(5, 0));

            if (ImGui::Button("Reload")) {
                _reload = true;
            }
            ImGui::Dummy(ImVec2(2, 0));
            if (ImGui::Button(Config::get()->Pause ? "Resume" : "Pause")) {
                Config::get()->Pause = !Config::get()->Pause;
            }

            ImGui::Dummy(ImVec2(2, 0));
            bool monitor = Config::get()->isMonitored();
            ImGui::Checkbox("Monitor", &monitor);
            Config::get()->setMonitored(monitor);

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
