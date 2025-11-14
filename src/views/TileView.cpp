#include "views/TileView.h"

#include "imgui.h"
#include "imgui_internal.h"

TileView::TileView(Naratte *naratte) : _naratte(naratte) {
}

void TileView::render() {
    render_tiles();
    render_tm();
}

void TileView::render_tiles() {
    ImGui::Begin("Tiles");

    ImGui::End();
}

void TileView::render_tm() {
    ImGui::Begin("Tile Maps");

    const int bank = _bank;
    if (bank == 0) ImGui::BeginDisabled();
    if (ImGui::Button("Bank 0")) _bank = 0;
    if (bank == 0) ImGui::EndDisabled();
    ImGui::SameLine();
    if (bank == 1) ImGui::BeginDisabled();
    if (ImGui::Button("Bank 1")) _bank = 1;
    if (bank == 1) ImGui::EndDisabled();

    ImGui::Separator();

    _naratte->enableMemChange(false);
    const int baseAddr = bank == 0 ? 0x9800 : 0x9C00;
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 3));
    if (ImGui::BeginTable("Background", 32))
    {
        for (int row = 0; row < 32; row++)
        {
            ImGui::TableNextRow();
            for (int column = 0; column < 32; column++)
            {
                const uint8_t vbk = _naratte->readPseudoMem(0xFF4F);

                _naratte->writePseudoMem(0xFF4F, 0);
                const int tileId = _naratte->readPseudoMem(baseAddr + row * 32 + column);

                _naratte->writePseudoMem(0xFF4F, 1);
                const uint8_t tileAttr = _naratte->readPseudoMem(baseAddr + row * 32 + column);

                ImGui::TableSetColumnIndex(column);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 0, (tileId * 16) % 255, 255));
                ImGui::Text("%d", tileId);

                if (ImGui::IsItemHovered())
                {
                    bool priority = (tileAttr >> 7) & 1;
                    bool yflip    = (tileAttr >> 6) & 1;
                    bool xflip    = (tileAttr >> 5) & 1;
                    bool bank     = (tileAttr >> 3) & 1;
                    uint8_t palette = tileAttr & 0b111; // bits 0–2

                    ImGui::BeginTooltip();

                    ImGui::Text("Tile: %u", tileId);
                    ImGui::Separator();

                    ImGui::Text("Attr byte: 0x%02X", tileAttr);
                    ImGui::Separator();

                    ImGui::Text("Bit 7: Priority       = %d", priority);
                    ImGui::Text("Bit 6: Y flip         = %d", yflip);
                    ImGui::Text("Bit 5: X flip         = %d", xflip);
                    ImGui::Text("Bit 4: (unused)");
                    ImGui::Text("Bit 3: Bank           = %d", bank);
                    ImGui::Text("Bit 2-0: Palette      = %u", palette);

                    ImGui::EndTooltip();
                }

                _naratte->writePseudoMem(0xFF4F, vbk);
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
    _naratte->enableMemChange(true);

    ImGui::End();
}
