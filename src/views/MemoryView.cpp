#include "views/MemoryView.h"

#include <cstdint>
#include <cstdio>

#include "imgui.h"

MemoryView::MemoryView(Naratte *naratte) : _naratte(naratte) {
}

void MemoryView::render() {
    ImGui::Begin("Memory");

    constexpr int mem_size = 0x10000;   // 64 KB
    constexpr int bytes_per_row = 16;

    // Fixed pitch font helps alignment
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {2, 2});
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {2, 1});
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // optional

    // Calculate total rows
    const int row_count = mem_size / bytes_per_row;

    // Text width per byte (hex + space)
    float char_width = ImGui::CalcTextSize("FF ").x;
    float addr_width = ImGui::CalcTextSize("0000: ").x;

    // Table
    if (ImGui::BeginTable("mem_table", 3,
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, addr_width);

        // Hex bytes
        ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthFixed,
            char_width * bytes_per_row);

        // ASCII side
        ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableHeadersRow();

        // Clip rows for performance
        ImGuiListClipper clipper;
        clipper.Begin(row_count);

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                uint16_t base_addr = row * bytes_per_row;

                ImGui::TableNextRow();

                // Address
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%04X:", base_addr);

                // Hex bytes
                ImGui::TableSetColumnIndex(1);
                {
                    char buf[bytes_per_row * 3 + 1];
                    char* out = buf;

                    for (int i = 0; i < bytes_per_row; i++)
                    {
                        const uint8_t value = _naratte->readPseudoMem(base_addr + i);
                        out += sprintf(out, "%02X ", value);
                    }
                    ImGui::TextUnformatted(buf);
                }

                // ASCII
                ImGui::TableSetColumnIndex(2);
                {
                    char buf[bytes_per_row + 1];
                    for (int i = 0; i < bytes_per_row; i++)
                    {
                        const uint8_t value = _naratte->readPseudoMem(base_addr + i);
                        buf[i] = (value >= 32 && value < 127) ? value : '.';
                    }
                    buf[bytes_per_row] = 0;
                    ImGui::TextUnformatted(buf);
                }
            }
        }

        ImGui::EndTable();
    }

    ImGui::PopFont();
    ImGui::PopStyleVar(2);

    ImGui::End();
}
