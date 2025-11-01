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

    // Store address input
    static char input_addr_buf[8] = "FFFF";
    static uint16_t scroll_to_addr = 0;
    static bool do_scroll = false;

    //---------------------------------------
    // Address Jump Field
    //---------------------------------------
    ImGui::Text("Jump to:");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(80);
    ImGui::InputText("##addr", input_addr_buf, sizeof(input_addr_buf),
                     ImGuiInputTextFlags_CharsHexadecimal |
                     ImGuiInputTextFlags_CharsUppercase);

    ImGui::SameLine();
    if (ImGui::Button("Jump"))
    {
        // Convert hex to address
        scroll_to_addr = (uint16_t)strtol(input_addr_buf, NULL, 16);
        do_scroll = true;
    }

    ImGui::Separator();

    //---------------------------------------
    // Memory Table
    //---------------------------------------
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {2, 2});
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {2, 1});
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // optional

    const int row_count = mem_size / bytes_per_row;

    // Approx line height (text height)
    float line_height = ImGui::GetTextLineHeightWithSpacing() + 3;

    if (ImGui::BeginTable("mem_table", 3,
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("Addr", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Data",     ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("ASCII",   ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        // Scroll handling: compute row index and target scroll position
        if (do_scroll)
        {
            uint32_t row = scroll_to_addr / bytes_per_row;
            float target_y = row * line_height;
            ImGui::SetScrollY(target_y + 18);
            do_scroll = false;
        }

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
                        if (i == 8)
                            out += sprintf(out, " ");
                        uint8_t value = _naratte->readPseudoMem(base_addr + i);
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
                        uint8_t value = _naratte->readPseudoMem(base_addr + i);
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
