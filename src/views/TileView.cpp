#include "views/TileView.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <GL/gl.h>

TileView::TileView(Naratte *naratte) : _naratte(naratte) {
    glGenTextures(1, &_tile_txt);
    glBindTexture(GL_TEXTURE_2D, _tile_txt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TileView::render() {
    update_tile();

    render_tiles();
    render_tm();
    render_obj();
    render_lcdc();
}

void TileView::render_tiles() {
    ImGui::Begin("Tiles");

    const float tex_width  = 256.0f;
    const float tex_height = 256.0f;
    const float tex_aspect = tex_width / tex_height;

    // available size inside the window
    ImVec2 avail = ImGui::GetContentRegionAvail();

    // scale while preserving aspect ratio
    float draw_width  = avail.x;
    float draw_height = avail.x / tex_aspect;
    if (draw_height > avail.y) {
        draw_height = avail.y;
        draw_width  = avail.y * tex_aspect;
    }

    // compute centered position
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 center_offset = ImVec2(
        (avail.x - draw_width) * 0.5f,
        (avail.y - draw_height) * 0.5f
    );

    // move cursor so image is centered
    ImGui::SetCursorScreenPos(ImVec2(cursor.x + center_offset.x,
                                     cursor.y + center_offset.y));

    ImGui::Image(_tile_txt,ImVec2(draw_width, draw_height));

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

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    const int addrMode = _addr_mode;
    if (addrMode == 0) ImGui::BeginDisabled();
    if (ImGui::Button("0x8000")) _addr_mode = 0;
    if (addrMode == 0) ImGui::EndDisabled();
    ImGui::SameLine();
    if (addrMode == 1) ImGui::BeginDisabled();
    if (ImGui::Button("0x8800")) _addr_mode = 1;
    if (addrMode == 1) ImGui::EndDisabled();

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
                if (ImGui::Selectable(
                    std::string(std::to_string(tileId) + "##" + std::to_string(column * 32 + row)).c_str(),
                    tileId == _selected && _tile_type == BG))
                    {
                    _selected = tileId;
                    _tile = tileId;
                    _tile_attr = tileAttr;
                    _tile_type = BG;
                }

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

                    ImGui::Text("x: %u   y: %u", column * 8, row * 8);
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

void TileView::render_obj() {
     ImGui::Begin("Objects");

    _naratte->enableMemChange(false);

    constexpr int baseAddr = 0xFE00;
    if (ImGui::BeginListBox("##listbox", ImVec2(-FLT_MIN, -FLT_MIN)))
    {
        for (int i = 0; i < 40; i++)
        {
            const uint16_t pos_y    = _naratte->readPseudoMem(baseAddr + i * 4 + 0) - 16;
            const uint16_t pos_x    = _naratte->readPseudoMem(baseAddr + i * 4 + 1) - 8;
            const uint8_t  obj_t_id = _naratte->readPseudoMem(baseAddr + i * 4 + 2);
            const uint8_t  attr     = _naratte->readPseudoMem(baseAddr + i * 4 + 3);

            if (pos_y > 256)
                continue;

            if (ImGui::Selectable(std::string("Object " + std::to_string(i)).c_str(), _selected == i && _tile_type == OBJ)) {
                _selected = i;
                _tile_type = OBJ;
                _tile = obj_t_id;
                _tile_attr = attr;
            }

            if (ImGui::IsItemHovered())
            {
                const bool priority = (attr >> 7) & 1;
                const bool yflip    = (attr >> 6) & 1;
                const bool xflip    = (attr >> 5) & 1;
                const bool dmg    = (attr >> 5) & 1;
                const bool bank     = (attr >> 3) & 1;
                uint8_t palette = attr & 0b111; // bits 0–2

                ImGui::BeginTooltip();

                ImGui::Text("Tile: %u", obj_t_id);
                ImGui::Separator();

                ImGui::Text("x: %u   y: %u", pos_x, pos_y);
                ImGui::Separator();

                ImGui::Text("Attr byte: 0x%02X", attr);
                ImGui::Separator();

                ImGui::Text("Bit 7: Priority       = %d", priority);
                ImGui::Text("Bit 6: Y flip         = %d", yflip);
                ImGui::Text("Bit 5: X flip         = %d", xflip);
                ImGui::Text("Bit 4: DMG palette	   = %d", dmg);
                ImGui::Text("Bit 3: Bank           = %d", bank);
                ImGui::Text("Bit 2-0: Palette      = %u", palette);

                ImGui::EndTooltip();
            }
        }

        ImGui::EndListBox();
    }

    _naratte->enableMemChange(true);

    ImGui::End();
}

void TileView::render_lcdc() {
    ImGui::Begin("LCDC Register");

    const uint8_t lcdc = _naratte->readPseudoMem(0xFF40);

    if (ImGui::BeginTable("lcdc_table", 2,
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Bit");
        ImGui::TableSetupColumn("Meaning");
        ImGui::TableHeadersRow();

        for (int bit = 7; bit >= 0; bit--)
        {
            bool set = lcdc & (1 << bit);

            ImVec4 green(0.2f, 1.0f, 0.2f, 1.0f);
            ImVec4 dim(0.6f, 0.6f, 0.6f, 1.0f);

            const ImVec4 &color = set ? green : dim;

            auto meaning = "";
            switch (bit)
            {
                case 7: meaning = set ? "LCD enabled" : "LCD disabled"; break;
                case 6: meaning = set ? "Window map = 0x9C00" : "Window map = 0x9800"; break;
                case 5: meaning = set ? "Window shown" : "Window hidden"; break;
                case 4: meaning = set ? "Tile data = 0x8000 (signed)" : "Tile data = 0x8800 (unsigned)"; break;
                case 3: meaning = set ? "BG map = 0x9C00" : "BG map = 0x9800"; break;
                case 2: meaning = set ? "8×16 sprites" : "8×8 sprites"; break;
                case 1: meaning = set ? "Sprites enabled" : "Sprites disabled"; break;
                case 0: meaning = set ? "BG/window enabled" : "BG/window disabled"; break;
                default: ;
            }

            ImGui::TableNextRow();

            // Column 0: bit number
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(color, "Bit %d", bit);

            // Column 1: meaning (safe format string!)
            ImGui::TableSetColumnIndex(1);
            ImGui::TextColored(color, "%s", meaning);
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void TileView::update_tile() {
    _naratte->enableMemChange(false);

    const uint8_t color_mode = (_naratte->readPseudoMem(0xFF4C) & 0x4) != 0x4;
    const uint8_t lcdc = _naratte->readPseudoMem(0xFF40);

    uint8_t fb[8*8]{};
    
    for (uint8_t y = 0; y < 8; y++) {
        const uint8_t y_flip = (_tile_attr & 0x40) ? (7 - y) : y;

        uint16_t tile_addr = (0x8000 + (_tile * 16)) + (y_flip * 2);
        if (_tile_type == BG && _addr_mode == 1)
            tile_addr = (0x9000 + (static_cast<int8_t>(_tile) * 16)) + (y_flip * 2);

        const uint8_t vbk = _naratte->readPseudoMem(0xFF4F);
        if ((_tile_attr & 0x08) && !((_tile_type == OBJ) && !color_mode))
            _naratte->writePseudoMem(0xFF4F, 1);
        else
            _naratte->writePseudoMem(0xFF4F, 0);
        const uint8_t lo = _naratte->readPseudoMem(tile_addr);
        const uint8_t hi = _naratte->readPseudoMem(tile_addr + 1);
        _naratte->writePseudoMem(0xFF4F, vbk);

        for (uint8_t x = 0; x < 8; x++) {
            const uint8_t bit = (_tile_attr & 0x20) ? x : (7 - x);  // X-flip
            uint8_t color = ((hi >> bit) & 1) << 1 | ((lo >> bit) & 1);


            /*if (!color_mode) {
                uint8_t palette = 0;
                if (_tile_type == OBJ)
                    palette = _naratte->readPseudoMem((_tile_attr & 0x10) ? 0xFF49 : 0xFF48) & ~0b11;
                else
                    palette = _naratte->readPseudoMem(0xFF47);
                color = (palette >> (color * 2)) & 3;
            }*/

            fb[y * 8 + x] = (color * 42) % 256;
        }
    }

    glBindTexture(GL_TEXTURE_2D, _tile_txt);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 8, 8, 0,
                 GL_RED, GL_UNSIGNED_BYTE, fb);
    glBindTexture(GL_TEXTURE_2D, 0);

    _naratte->enableMemChange(true);
}
