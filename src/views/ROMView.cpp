#include "views/ROMView.h"

#include <filesystem>
#include <fstream>

#include "Config.h"
#include "imgui.h"

ROMView::ROMView(Naratte *naratte) : _naratte(naratte){
    load_roms();
}

void ROMView::render() {
    ImGui::Begin("ROMs");

    if (ImGui::Button("Reload ROM List")) {
        load_roms();
    }

    ImGui::Separator();

    if (ImGui::BeginListBox("##listbox", ImVec2(-FLT_MIN, -FLT_MIN)))
    {
        for (int i = 0; i < _roms.size(); i++)
        {
            if (ImGui::Selectable(_roms[i].c_str(), i == _selected)) {
                if (_selected != i) {
                    Config::get()->settings.pathRom = _roms[i];
                    Config::get()->save();
                }

                _selected = i;
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                Config::get()->settings.pathRom = _roms[i];
                Config::get()->Reload = true;
                Config::get()->save();
            }
        }

        ImGui::EndListBox();
    }

    ImGui::End();
}

void ROMView::load_roms() {
    _roms.clear();

    const std::string path = "roms.conf";
    if (!std::filesystem::exists(path))
        return;
    std::ifstream file(path);
    if (!file.is_open())
        return;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty())
            continue;
        if (std::filesystem::is_regular_file(line))
            _roms.push_back(line);
    }
}
