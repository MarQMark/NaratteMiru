#include "views/ExportView.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

ExportView::ExportView(Naratte *naratte) : _naratte(naratte){
    _visible = false;
}

void ExportView::render() {
    ImGui::OpenPopup("Export");

    if (ImGui::BeginPopupModal("Export", nullptr))
    {
        ImGui::Text("Path");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("###Path", &_path);

        ImGui::Text("Name");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("###Name", &_name);

        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));

        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            _visible = false;
        }
        ImGui::SameLine();

        const bool enabled = !_path.empty() && !_name.empty();
        if (!enabled)
            ImGui::BeginDisabled();
        if (ImGui::Button("Export")) {
            ImGui::CloseCurrentPopup();
            _visible = false;
            _naratte->serialize(_path + _name);
        }
        if (!enabled)
            ImGui::EndDisabled();

        ImGui::EndPopup();
    }
}
