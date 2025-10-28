#include "views/AsmView.h"

#include "imgui.h"

AsmView::AsmView(Naratte* naratte) : _naratte(naratte){
}

void AsmView::render() {
    ImGui::Begin("Assembly");

    if (ImGui::BeginListBox("##instr_list", ImVec2(-FLT_MIN, -FLT_MIN))) {
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(_naratte->getInstructions().size()));

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                const bool is_selected = (_selected == i);
                char label[128];
                auto& instruction = _naratte->getInstructions()[i];
                snprintf(label, sizeof(label), "%06d | %02X %02X %02X | %s",
                         i, instruction.op[0], instruction.op[1],
                         instruction.op[2], _naratte->getInstructionName(instruction.op));
                if (ImGui::Selectable(label, is_selected))
                    _selected = i;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndListBox();
    }

    ImGui::End();
}
