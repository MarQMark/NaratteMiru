#include "views/RegisterView.h"

#include "imgui.h"

RegisterView::RegisterView(Naratte *naratte) : _naratte(naratte){
}

void RegisterView::render() {
    ImGui::Begin("Registers");

    if (_naratte->getSelected() == -1) {
        ImGui::End();
        return;
    }

    const auto& cpu = _naratte->getInstructions()[_naratte->getSelected()].cpu;

    ImGui::SeparatorText("Registers");

    ImGui::BeginGroup();
    ImGui::Text("AF  %04X   A %02X   F %02X", cpu.AF, cpu.A, cpu.F);
    ImGui::Text("BC  %04X   B %02X   C %02X", cpu.BC, cpu.B, cpu.C);
    ImGui::Text("DE  %04X   D %02X   E %02X", cpu.DE, cpu.D, cpu.E);
    ImGui::Text("HL  %04X   H %02X   L %02X", cpu.HL, cpu.H, cpu.L);
    ImGui::EndGroup();

    ImGui::Dummy({0, 4});

    // --- FLAGS --------------------------------------------------------------
    ImGui::SeparatorText("Flags");

    auto flag = [&](char name, bool on)
    {
        if (on) {
            ImGui::TextColored(ImVec4(0,1,0,1), "%c", name);
        } else {
            ImGui::TextDisabled("%c", name);
        }
        ImGui::SameLine();
    };

    flag('Z', (cpu.F >> 7) & 1);
    flag('N', (cpu.F >> 6) & 1);
    flag('H', (cpu.F >> 5) & 1);
    flag('C', (cpu.F >> 4) & 1);
    ImGui::NewLine();

    ImGui::Dummy({0, 4});

    // --- POINTERS ----------------------------------------------------------
    ImGui::SeparatorText("Pointers");
    ImGui::Text("PC  %04X", cpu.PC);
    ImGui::Text("SP  %04X", cpu.SP);

    // --- IME ---------------------------------------------------------------
    ImGui::SeparatorText("Control");
    ImGui::Text("IME %u", cpu.IME);

    ImGui::End();
}
