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
                char* name = _naratte->getInstructionName(instruction.op);

                char op1[4], op2[4];
                snprintf(op1, sizeof(op1), "%02X", instruction.op[0]);

                // only if opcount ≥ 2, otherwise spaces
                if (instruction.op[3] >= 2)
                    snprintf(op2, sizeof(op2), "%02X", instruction.op[1]);
                else
                    snprintf(op2, sizeof(op2), "  ");

                // only if opcount ≥ 3, otherwise spaces
                char op3[4];
                if (instruction.op[3] >= 3)
                    snprintf(op3, sizeof(op3), "%02X", instruction.op[2]);
                else
                    snprintf(op3, sizeof(op3), "  ");

                snprintf(label, sizeof(label), "%06d | %s %s %s | %s",
                         i, op1, op2, op3, name);

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