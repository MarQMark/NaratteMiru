#include "views/SettingsView.h"

#include "Config.h"
#include "imgui.h"
#include "Util.h"
#include "misc/cpp/imgui_stdlib.h"

SettingsView::SettingsView() {
    _visible = false;
}

static const char* sSubmenuNames[] = {
    "General",
    "Formatting",
};
static constexpr int sSubmenuCount = sizeof(sSubmenuNames) / sizeof(sSubmenuNames[0]);

void SettingsView::render() {
    ImGui::OpenPopup("Settings");

    if (ImGui::BeginPopupModal("Settings", nullptr))
    {
        const auto childHeight = ImGui::GetContentRegionAvail().y - 30;

        ImGui::BeginChild("##left", ImVec2(100.0f, childHeight), true);
        {
            for (int i = 0; i < sSubmenuCount; i++)
            {
                if (ImGui::Selectable(sSubmenuNames[i], _menu == i))
                    _menu = i;
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##right", ImVec2(ImGui::GetContentRegionAvail().x, childHeight), true);
        {
            switch (_menu)
            {
                case 0: menu_general(); break;
                case 1: menu_format(); break;
                default: break;
            }
        }
        ImGui::EndChild();

        ImGui::Separator();

        constexpr float buttonW = 70;
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - (buttonW * 3 + ImGui::GetStyle().ItemSpacing.x));
        if (ImGui::Button("OK", ImVec2(buttonW, 0))) {
            apply();
            _visible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(buttonW, 0))) {
            _visible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply",ImVec2(buttonW, 0))) {
            apply();
        }

        ImGui::EndPopup();
    }
}

void SettingsView::setVisible(const bool visible) {
    if (!_visible && visible) {
        _lib_path = Config::get()->getLibPath();
        _boot_path = Config::get()->getBootPath();
        _game_path = Config::get()->getGamePath();
    }

    Viewable::setVisible(visible);
}

void SettingsView::menu_general() {
    const auto inputH = ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2.0f;
    const auto inputW = ImGui::GetContentRegionAvail().x - (inputH + 10);

    ImGui::Text("Narrate Path");
    ImGui::SetNextItemWidth(inputW);
    ImGui::InputText("###LibPathInput", &_lib_path);
    ImGui::SameLine();
    if (ImGui::Button("O###BtnLib", ImVec2(inputH, inputH))) {
        if (const auto path = Util::OpenExplorer("*.so *.dll", "Select Naratte Library"); !path.empty())
            _lib_path = path;
    }

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 5));

    ImGui::Text("Boot ROM Path");
    ImGui::SetNextItemWidth(inputW);
    ImGui::InputText("###BootPathInput", &_boot_path);
    ImGui::SameLine();
    if (ImGui::Button("O###BtnBoot", ImVec2(inputH, inputH))) {
        if (const auto path = Util::OpenExplorer("*.bin", "Select Boot ROM"); !path.empty())
            _boot_path = path;
    }

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::Text("Game ROM Path");
    ImGui::SetNextItemWidth(inputW);
    ImGui::InputText("###GamePathInput", &_game_path);
    ImGui::SameLine();
    if (ImGui::Button("O###BtnGame", ImVec2(inputH, inputH))) {
        if (const auto path = Util::OpenExplorer("*.gb *.gbc", "Select Game ROM"); !path.empty())
            _game_path = path;
    }
}

void SettingsView::menu_format() {
}

void SettingsView::apply() const {
    const auto config = Config::get();
    config->setLibPath(_lib_path);
    config->setBootPath(_boot_path);
    config->setGamePath(_game_path);
    config->save();
}
