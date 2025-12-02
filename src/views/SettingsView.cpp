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
    "Emulation",
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
                case 1: menu_emulation(); break;
                case 2: menu_format(); break;
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

        const bool dirty = _dirty;
        if (!dirty)
            ImGui::BeginDisabled();
        if (ImGui::Button("Apply",ImVec2(buttonW, 0))) {
            apply();
        }
        if (!dirty)
            ImGui::EndDisabled();

        ImGui::EndPopup();
    }
}

void SettingsView::setVisible(const bool visible) {
    if (!_visible && visible) {
        _auto_reload = Config::get()->getAutoReload();
        _lib_path = Config::get()->getLibPath();
        _boot_path = Config::get()->getBootPath();
        _game_path = Config::get()->getGamePath();
        _endless_loop = Config::get()->isEndlessLoop();
        _min_fr = Config::get()->getMinFR();
        _speed_multi = Config::get()->getSpeedMulti();
        _jp_as_call = Config::get()->getJPasCALL();
        _jac_start = Config::get()->getJaCStart();
        _jac_end = Config::get()->getJaCEnd();
    }

    Viewable::setVisible(visible);
}

void SettingsView::menu_general() {
    const auto inputH = ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2.0f;
    const auto inputW = ImGui::GetContentRegionAvail().x - (inputH + 10);

    ImGui::Text("Narrate Path");
    ImGui::SetNextItemWidth(inputW);
    const std::string libPathShdw = _lib_path;
    ImGui::InputText("###LibPathInput", &_lib_path);
    ImGui::SameLine();
    if (ImGui::Button("O###BtnLib", ImVec2(inputH, inputH))) {
        if (const auto path = Util::OpenExplorer("*.so *.dll", "Select Naratte Library"); !path.empty())
            _lib_path = path;
    }
    if (libPathShdw != _lib_path) _dirty = true;

    ImGui::Dummy(ImVec2(0, 10));
    const bool autoReloadShdw = _auto_reload;
    ImGui::Checkbox("###auto-reload", &_auto_reload);
    ImGui::SameLine();
    ImGui::Text("Auto-Reload");
    if (autoReloadShdw != _auto_reload) _dirty = true;

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 15));

    ImGui::Text("Boot ROM Path");
    ImGui::SetNextItemWidth(inputW);
    const std::string bootPathShdw = _boot_path;
    ImGui::InputText("###BootPathInput", &_boot_path);
    ImGui::SameLine();
    if (ImGui::Button("O###BtnBoot", ImVec2(inputH, inputH))) {
        if (const auto path = Util::OpenExplorer("*.bin", "Select Boot ROM"); !path.empty())
            _boot_path = path;
    }
    if (bootPathShdw != _boot_path) _dirty = true;

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::Text("Game ROM Path");
    ImGui::SetNextItemWidth(inputW);
    const std::string gamePathShdw = _game_path;
    ImGui::InputText("###GamePathInput", &_game_path);
    ImGui::SameLine();
    if (ImGui::Button("O###BtnGame", ImVec2(inputH, inputH))) {
        if (const auto path = Util::OpenExplorer("*.gb *.gbc", "Select Game ROM"); !path.empty())
            _game_path = path;
    }
    if (gamePathShdw != _game_path) _dirty = true;

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Separator();
}

void SettingsView::menu_emulation() {
    const bool endlessLoopShdw = _endless_loop;
    ImGui::Checkbox("##endlessLoop", &_endless_loop);
    ImGui::SameLine();
    ImGui::Text("Stop when endless loop detected");
    if (endlessLoopShdw != _endless_loop) _dirty = true;

    const double speedMultiShdw = _speed_multi;
    std::string speedMulti = std::to_string(_speed_multi);
    ImGui::InputText("Minimum Framerate (Hz)", &speedMulti, ImGuiInputTextFlags_CharsScientific);
    try {
        _speed_multi = std::stod(speedMulti);
    } catch (...) {}
    if (speedMultiShdw != _speed_multi) _dirty = true;

    const float minFRShdw = _min_fr;
    std::string minFR = std::to_string(_min_fr);
    ImGui::InputText("Speed Multiplier", &minFR, ImGuiInputTextFlags_CharsScientific);
    try {
        _min_fr = std::stof(minFR);
    } catch (...) {}
    if (minFRShdw != _min_fr) _dirty = true;

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Separator();
}

void SettingsView::menu_format() {
    ImGui::Text("Treat JP as CALL:");
    const bool JPasCallShdw = _jp_as_call;
    ImGui::Checkbox("##$JAC", &_jp_as_call);
    if (JPasCallShdw != _jp_as_call) _dirty = true;

    const auto inputW = (ImGui::GetContentRegionAvail().x - 20) / 3;

    if (!_jp_as_call)
        ImGui::BeginDisabled();

    ImGui::SameLine();
    ImGui::SetNextItemWidth(inputW);
    const int jacStartShdw = _jac_start;
    std::string start = std::to_string(_jac_start);
    ImGui::InputText("Start", &start, ImGuiInputTextFlags_CharsDecimal);
    try {
        _jac_start = std::stoi(start);
    } catch (...) {}
    if (jacStartShdw != _jac_start) _dirty = true;

    ImGui::SameLine();
    ImGui::SetNextItemWidth(inputW);
    const int jacEndShdw = _jac_end;
    std::string end = std::to_string(_jac_end);
    ImGui::InputText("End", &end, ImGuiInputTextFlags_CharsDecimal);
    try {
        _jac_end = std::stoi(end);
    } catch (...) {}
    if (jacEndShdw != _jac_end) _dirty = true;

    if (!_jp_as_call)
        ImGui::EndDisabled();

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Separator();
}

void SettingsView::apply() {
    _dirty = false;

    const auto config = Config::get();
    config->setAutoReload(_auto_reload);
    config->setLibPath(_lib_path);
    config->setBootPath(_boot_path);
    config->setGamePath(_game_path);

    config->setEndlessLoop(_endless_loop);
    config->setMinFR(_min_fr);
    config->setSpeedMulti(_speed_multi);

    config->setJPasCALL(_jp_as_call);
    config->setJaCStart(_jac_start);
    config->setJaCEnd(_jac_end);

    config->save();
}
