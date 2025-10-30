#include "views/AsmView.h"

#include "imgui.h"

AsmView::AsmView(Naratte* naratte) : _naratte(naratte){
}

void AsmView::render() {
    int oldSelected = _selected;

    if (_selected > _naratte->getInstructions().size())
        _selected = -1;

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

    if (oldSelected != _selected)
        _naratte->reloadPseudoMem(_selected);

    /*ImGui::Begin("Mem Writes");

    if (ImGui::BeginListBox("##instr_lis2t", ImVec2(-FLT_MIN, -FLT_MIN))) {
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(_naratte->getMemWrites().size()));
        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                auto& write = _naratte->getMemWrites()[i];
                const bool is_selected = (_selected == write.idx);
                char label[128];
                snprintf(label, sizeof(label), "%06lu | %04X %02X",
                         write.idx, write.addr, write.data);
                if (ImGui::Selectable(label, is_selected))
                    _selected = write.idx;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndListBox();
    }

    ImGui::End();*/
}