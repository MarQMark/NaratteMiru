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
                add_menu_view("Tiles", &_tile_view);
                add_menu_view("Roms", &_rom_view);

                ImGui::EndMenu();
            }

            ImGui::Dummy(ImVec2(1, 0));
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::Dummy(ImVec2(5, 0));

            if (ImGui::Button("Reload")) {
                Config::get()->Reload = true;
            }
            ImGui::Dummy(ImVec2(2, 0));
            if (ImGui::Button(Config::get()->Pause ? "Resume" : "Pause")) {
                Config::get()->Pause = !Config::get()->Pause;
            }

            ImGui::Dummy(ImVec2(2, 0));
            bool monitor = Config::get()->properties.monitoring.get();
            ImGui::Checkbox("Monitor", &monitor);
            Config::get()->properties.monitoring = monitor;

            ImGui::Dummy(ImVec2(1, 0));
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::Dummy(ImVec2(5, 0));

            if (ImGui::Button("Save Snapshot")) {
                _naratte->saveSnapshot();
            }
            if (ImGui::Button("Load Snapshot")) {
                _naratte->loadSnapshot();
            }


            {
                const char* letters[] = { "S", "s", "B", "A", "D", "U", "L", "R" };
                ImGuiKey keys[] = {
                    ImGuiKey_T, ImGuiKey_Y, ImGuiKey_L, ImGuiKey_K,
                    ImGuiKey_S, ImGuiKey_W, ImGuiKey_A, ImGuiKey_D
                };

                const ImGuiStyle& style = ImGui::GetStyle();

                constexpr float spacing = 8.0f;
                float total_width = 0.0f;
                for (int i = 0; i < IM_ARRAYSIZE(letters); ++i) {
                    const ImVec2 ts = ImGui::CalcTextSize(letters[i]);
                    total_width += ts.x;
                    if (i + 1 < IM_ARRAYSIZE(letters)) total_width += spacing;
                }

                const float window_width = ImGui::GetWindowWidth();
                const float padding_right = style.FramePadding.x + 8.0f;
                const float start_x = window_width - total_width - padding_right;
                if (start_x > ImGui::GetCursorPosX()) {
                    ImGui::SameLine(start_x);
                } else {
                    ImGui::SameLine();
                }

                constexpr auto green = ImVec4(0.2f, 0.85f, 0.2f, 1.0f);
                const ImVec4 default_col = ImGui::GetStyleColorVec4(ImGuiCol_Text);

                for (int i = 0; i < IM_ARRAYSIZE(letters); ++i) {
                    const bool pressed = ImGui::IsKeyDown(keys[i]);

                    if (pressed)
                        Config::get()->Joypad |=  (1u << (7 - i));
                    else
                        Config::get()->Joypad &= ~(1u << (7 - i));

                    ImVec4 col = pressed ? green : default_col;
                    ImGui::TextColored(col, "%s", letters[i]);

                    if (i + 1 < IM_ARRAYSIZE(letters)) {
                        ImGui::SameLine(0.0f, spacing);
                    }
                }
            }

            ImGui::EndMenuBar();
        }
        ImGui::End();
    }
}

void Menubar::add_menu_view(const std::string &name, bool *enabled) const {
    if(ImGui::MenuItem(name.c_str(), nullptr, enabled))
        static_cast<View *>(view)->getViewable(name)->setVisible(*enabled);
}
