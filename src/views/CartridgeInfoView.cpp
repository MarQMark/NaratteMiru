#include "views/CartridgeInfoView.h"

#include "imgui.h"

CartridgeInfoView::CartridgeInfoView(Naratte *naratte) : _naratte(naratte){
    _visible = false;
}

void CartridgeInfoView::render() {
    ImGui::OpenPopup("Cartridge Info");

    if (ImGui::BeginPopupModal("Cartridge Info", nullptr)) {

        bool cgb = read_mem(0x0143) == 0x80 || read_mem(0x0143) == 0xC0;

        ImGui::Text("Title [0134, 0143]");
        char title[17]{};
        for (int i = 0; i < 17; i++) {
            title[i] = read_mem(0x0134 + i);
            if (!isascii(title[i])) break;
        }
        ImGui::Text("  %s", title);
        ImGui::Dummy(ImVec2(0, 5));

        if (cgb) {
            ImGui::Text("Manufacturer Code [013F, 0142]");
            ImGui::Text("  %02X %02X %02X %02X", read_mem(0x013F), read_mem(0x0140), read_mem(0x0141), read_mem(0x0142));
            ImGui::Dummy(ImVec2(0, 5));

            ImGui::Text("CBC Flag [0143]");
            ImGui::Text("  %02X:", read_mem(0x0143));
            ImGui::SameLine();
            if (read_mem(0x0143) == 0x80)
                ImGui::Text("Supports CBG, backwards compatible");
            else if (read_mem(0x0143) == 0xC0)
                ImGui::Text("CBG only");
            ImGui::Dummy(ImVec2(0, 5));
        }

        if (read_mem(0x014B) == 0x33) {
            ImGui::Text("New licensee code [0144, 0145]");
            std::string code;
            code += read_mem(0x0144);
            code += read_mem(0x0145);
            if (const auto it = _new_licensee.find(code); it != _new_licensee.end()) {
                ImGui::Text("  %c %c: %s", code[0], code[1], it->second);
            } else {
                ImGui::Text("  %c %c: Unknown", code[0], code[1]);
            }

            ImGui::Dummy(ImVec2(0, 5));
        }

        ImGui::Text("SGB Flag [0146]");
        ImGui::Text("  %02X", read_mem(0x0146));
        ImGui::Dummy(ImVec2(0, 5));

        ImGui::Text("Cartridge Type [0147]");
        ImGui::Text("  %02X: %s", read_mem(0x0147), cartridge_type(read_mem(0x0147)));
        ImGui::Dummy(ImVec2(0, 5));

        ImGui::Text("ROM size [0148]");
        ImGui::Text("  %02X: %s", read_mem(0x0148), rom_size(read_mem(0x0148)));
        ImGui::Dummy(ImVec2(0, 5));

        ImGui::Text("RAM size [0149]");
        ImGui::Text("  %02X: %s", read_mem(0x0149), rom_size(read_mem(0x0149)));
        ImGui::Dummy(ImVec2(0, 5));

        ImGui::Text("Destination code [014A]");
        ImGui::Text("  %02X: %s", read_mem(0x014A), read_mem(0x014A) == 0 ? "Japan" : read_mem(0x014A) == 1 ? "Overseas only" : "Invalid");
        ImGui::Dummy(ImVec2(0, 5));

        ImGui::Text("Old licensee code [014B]");
        ImGui::Text("  %02X: %s", read_mem(0x014B), old_licensee_code(read_mem(0x014B)));
        ImGui::Dummy(ImVec2(0, 5));

        ImGui::Text("Mask ROM version number [014C]");
        ImGui::Text("  %02X", read_mem(0x014C));
        ImGui::Dummy(ImVec2(0, 5));

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 23);
        ImGui::Separator();
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x / 2 - 35);
        if (ImGui::Button("Close", ImVec2(80, 0))) {
            _visible = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

uint8_t CartridgeInfoView::read_mem(const uint16_t addr) const {
    return _naratte->readPseudoMem(addr);
}

const char* CartridgeInfoView::cartridge_type(const uint8_t type) {
    switch (type) {
        case 0x00: return "ROM only";
        case 0x01: return "MBC1";
        case 0x02: return "MBC1+RAM";
        case 0x03: return "MBC1+RAM+BATTERY";
        case 0x05: return "MBC2";
        case 0x06: return "MBC2+BATTERY";
        case 0x08: return "ROM+RAM";
        case 0x09: return "ROM+RAM+BATTERY";
        case 0x0B: return "MMM01";
        case 0x0C: return "MMM01+RAM";
        case 0x0D: return "MMM01+RAM+BATTERY";
        case 0x0F: return "MBC3+TIMER+BATTERY";
        case 0x10: return "MBC3+TIMER+RAM+BATTERY";
        case 0x11: return "MBC3";
        case 0x12: return "MBC3+RAM";
        case 0x13: return "MBC3+RAM+BATTERY";
        case 0x19: return "MBC5";
        case 0x1A: return "MBC5+RAM";
        case 0x1B: return "MBC5+RAM+BATTERY";
        case 0x1C: return "MBC5+RUMBLE";
        case 0x1D: return "MBC5+RUMBLE+RAM";
        case 0x1E: return "MBC5+RUMBLE+RAM+BATTERY";
        case 0x20: return "MBC6";
        case 0x22: return "MBC7+SENSOR+RUMBLE+RAM+BATTERY";
        case 0xFC: return "POCKET CAMERA";
        case 0xFD: return "BANDAI TAMA5";
        case 0xFE: return "HuC3";
        case 0xFF: return "HuC1+RAM+BATTERY";
        default: return "Invalid";
    }
}

const char * CartridgeInfoView::rom_size(const uint8_t type) {
    switch (type) {
        case 0x00: return "Size: 32KiB,   Num Banks: 2 (no banking)";
        case 0x01: return "Size: 64KiB,   Num Banks: 4";
        case 0x02: return "Size: 128KiB,  Num Banks: 8";
        case 0x03: return "Size: 256KiB,  Num Banks: 16";
        case 0x04: return "Size: 512KiB,  Num Banks: 32";
        case 0x05: return "Size: 1MiB,    Num Banks: 64";
        case 0x06: return "Size: 2MiB,    Num Banks: 128";
        case 0x07: return "Size: 4MiB,    Num Banks: 256";
        case 0x08: return "Size: 8MiB,    Num Banks: 512";
        case 0x52: return "Size: 1.1MiB,  Num Banks: 72 (likely wrong)";
        case 0x53: return "Size: 1.2MiB,  Num Banks: 80 (likely wrong)";
        case 0x54: return "Size: 1.5MiB,  Num Banks: 96 (likely wrong)";
        default: return "Invalid";
    }
}

const char * CartridgeInfoView::ram_size(const uint8_t type) {
    switch (type) {
        case 0x00: return "Size: 0,      Num Banks: No RAM";
        case 0x01: return "Size: -,      Num Banks: Unused*";
        case 0x02: return "Size: 8KiB,   Num Banks: 1";
        case 0x03: return "Size: 32KiB,  Num Banks: 4";
        case 0x04: return "Size: 128KiB, Num Banks: 16";
        case 0x05: return "Size: 64KiB,  Num Banks: 8";
        default: return "Invalid";
    }
}

const char * CartridgeInfoView::old_licensee_code(const uint8_t type) {
    switch (type) {
        case(0x00): return	"None";
        case(0x01): return	"Nintendo";
        case(0x08): return	"Capcom";
        case(0x09): return	"HOT-B";
        case(0x0A): return	"Jaleco";
        case(0x0B): return	"Coconuts Japan";
        case(0x0C): return	"Elite Systems";
        case(0x13): return	"EA (Electronic Arts)";
        case(0x18): return	"Hudson Soft";
        case(0x19): return	"ITC Entertainment";
        case(0x1A): return	"Yanoman";
        case(0x1D): return	"Japan Clary";
        case(0x1F): return	"Virgin Games Ltd.";
        case(0x24): return	"PCM Complete";
        case(0x25): return	"San-X";
        case(0x28): return	"Kemco";
        case(0x29): return	"SETA Corporation";
        case(0x30): return	"Infogrames";
        case(0x31): return	"Nintendo";
        case(0x32): return	"Bandai";
        case(0x33): return	"New licensee code should be used instead.";
        case(0x34): return	"Konami";
        case(0x35): return	"HectorSoft";
        case(0x38): return	"Capcom";
        case(0x39): return	"Banpresto";
        case(0x3C): return	"Entertainment Interactive (stub)";
        case(0x3E): return	"Gremlin";
        case(0x41): return	"Ubi Soft";
        case(0x42): return	"Atlus";
        case(0x44): return	"Malibu Interactive";
        case(0x46): return	"Angel";
        case(0x47): return	"Spectrum HoloByte";
        case(0x49): return	"Irem";
        case(0x4A): return	"Virgin Games Ltd.";
        case(0x4D): return	"Malibu Interactive";
        case(0x4F): return	"U.S. Gold";
        case(0x50): return	"Absolute";
        case(0x51): return	"Acclaim Entertainment";
        case(0x52): return	"Activision";
        case(0x53): return	"Sammy USA Corporation";
        case(0x54): return	"GameTek";
        case(0x55): return	"Park Place";
        case(0x56): return	"LJN";
        case(0x57): return	"Matchbox";
        case(0x59): return	"Milton Bradley Company";
        case(0x5A): return	"Mindscape";
        case(0x5B): return	"Romstar";
        case(0x5C): return	"Naxat Soft";
        case(0x5D): return	"Tradewest";
        case(0x60): return	"Titus Interactive";
        case(0x61): return	"Virgin Games Ltd.";
        case(0x67): return	"Ocean Software";
        case(0x69): return	"EA (Electronic Arts)";
        case(0x6E): return	"Elite Systems";
        case(0x6F): return	"Electro Brain";
        case(0x70): return	"Infogrames";
        case(0x71): return	"Interplay Entertainment";
        case(0x72): return	"Broderbund";
        case(0x73): return	"Sculptured Software";
        case(0x75): return	"The Sales Curve Limited";
        case(0x78): return	"THQ";
        case(0x79): return	"Accolade";
        case(0x7A): return	"Triffix Entertainment";
        case(0x7C): return	"MicroProse";
        case(0x7F): return	"Kemco";
        case(0x80): return	"Misawa Entertainment";
        case(0x83): return	"LOZC G.";
        case(0x86): return	"Tokuma Shoten";
        case(0x8B): return	"Bullet-Proof Software";
        case(0x8C): return	"Vic Tokai Corp.";
        case(0x8E): return	"Ape Inc.";
        case(0x8F): return	"I’Max";
        case(0x91): return	"Chunsoft Co.";
        case(0x92): return	"Video System";
        case(0x93): return	"Tsubaraya Productions";
        case(0x95): return	"Varie";
        case(0x96): return	"Yonezawa/S’Pal";
        case(0x97): return	"Kemco";
        case(0x99): return	"Arc";
        case(0x9A): return	"Nihon Bussan";
        case(0x9B): return	"Tecmo";
        case(0x9C): return	"Imagineer";
        case(0x9D): return	"Banpresto";
        case(0x9F): return	"Nova";
        case(0xA1): return	"Hori Electric";
        case(0xA2): return	"Bandai";
        case(0xA4): return	"Konami";
        case(0xA6): return	"Kawada";
        case(0xA7): return	"Takara";
        case(0xA9): return	"Technos Japan";
        case(0xAA): return	"Broderbund";
        case(0xAC): return	"Toei Animation";
        case(0xAD): return	"Toho";
        case(0xAF): return	"Namco";
        case(0xB0): return	"Acclaim Entertainment";
        case(0xB1): return	"ASCII Corporation or Nexsoft";
        case(0xB2): return	"Bandai";
        case(0xB4): return	"Square Enix";
        case(0xB6): return	"HAL Laboratory";
        case(0xB7): return	"SNK";
        case(0xB9): return	"Pony Canyon";
        case(0xBA): return	"Culture Brain";
        case(0xBB): return	"Sunsoft";
        case(0xBD): return	"Sony Imagesoft";
        case(0xBF): return	"Sammy Corporation";
        case(0xC0): return	"Taito";
        case(0xC2): return	"Kemco";
        case(0xC3): return	"Square";
        case(0xC4): return	"Tokuma Shoten";
        case(0xC5): return	"Data East";
        case(0xC6): return	"Tonkin House";
        case(0xC8): return	"Koei";
        case(0xC9): return	"UFL";
        case(0xCA): return	"Ultra Games";
        case(0xCB): return	"VAP, Inc.";
        case(0xCC): return	"Use Corporation";
        case(0xCD): return	"Meldac";
        case(0xCE): return	"Pony Canyon";
        case(0xCF): return	"Angel";
        case(0xD0): return	"Taito";
        case(0xD1): return	"SOFEL (Software Engineering Lab)";
        case(0xD2): return	"Quest";
        case(0xD3): return	"Sigma Enterprises";
        case(0xD4): return	"ASK Kodansha Co.";
        case(0xD6): return	"Naxat Soft16";
        case(0xD7): return	"Copya System";
        case(0xD9): return	"Banpresto";
        case(0xDA): return	"Tomy";
        case(0xDB): return	"LJN";
        case(0xDD): return	"Nippon Computer Systems";
        case(0xDE): return	"Human Ent.";
        case(0xDF): return	"Altron";
        case(0xE0): return	"Jaleco";
        case(0xE1): return	"Towa Chiki";
        case(0xE2): return	"Yutaka";
        case(0xE3): return	"Varie";
        case(0xE5): return	"Epoch";
        case(0xE7): return	"Athena";
        case(0xE8): return	"Asmik Ace Entertainment";
        case(0xE9): return	"Natsume";
        case(0xEA): return	"King Records";
        case(0xEB): return	"Atlus";
        case(0xEC): return	"Epic/Sony Records";
        case(0xEE): return	"IGS";
        case(0xF0): return	"A Wave";
        case(0xF3): return	"Extreme Entertainment";
        case(0xFF): return	"LJN";
        default: return "Unknown";
    }
}

std::unordered_map<std::string, const char*> CartridgeInfoView::_new_licensee = {
    {"00",	"None"},
    {"01",	"Nintendo Research & Development 1"},
    {"08",	"Capcom"},
    {"13",	"EA (Electronic Arts)"},
    {"18",	"Hudson Soft"},
    {"19",	"B-AI"},
    {"20",	"KSS"},
    {"22",	"Planning Office WADA"},
    {"24",	"PCM Complete"},
    {"25",	"San-X"},
    {"28",	"Kemco"},
    {"29",	"SETA Corporation"},
    {"30",	"Viacom"},
    {"31",	"Nintendo"},
    {"32",	"Bandai"},
    {"33",	"Ocean Software/Acclaim Entertainment"},
    {"34",	"Konami"},
    {"35",	"HectorSoft"},
    {"37",	"Taito"},
    {"38",	"Hudson Soft"},
    {"39",	"Banpresto"},
    {"41",	"Ubi Soft"},
    {"42",	"Atlus"},
    {"44",	"Malibu Interactive"},
    {"46",	"Angel"},
    {"47",	"Bullet-Proof Software"},
    {"49",	"Irem"},
    {"50",	"Absolute"},
    {"51",	"Acclaim Entertainment"},
    {"52",	"Activision"},
    {"53",	"Sammy USA Corporation"},
    {"54",	"Konami"},
    {"55",	"Hi Tech Expressions"},
    {"56",	"LJN"},
    {"57",	"Matchbox"},
    {"58",	"Mattel"},
    {"59",	"Milton Bradley Company"},
    {"60",	"Titus Interactive"},
    {"61",	"Virgin Games Ltd."},
    {"64",	"Lucasfilm Games"},
    {"67",	"Ocean Software"},
    {"69",	"EA (Electronic Arts)"},
    {"70",	"Infogrames5"},
    {"71",	"Interplay Entertainment"},
    {"72",	"Broderbund"},
    {"73",	"Sculptured Software"},
    {"75",	"The Sales Curve Limited7"},
    {"78",	"THQ"},
    {"79",	"Accolade"},
    {"80",	"Misawa Entertainment"},
    {"83",	"LOZC G."},
    {"86",	"Tokuma Shoten"},
    {"87",	"Tsukuda Original"},
    {"91",	"Chunsoft Co."},
    {"92",	"Video System"},
    {"93",	"Ocean Software/Acclaim Entertainment"},
    {"95",	"Varie"},
    {"96",	"Yonezawa/S’Pal"},
    {"97",	"Kaneko"},
    {"99",	"Pack-In-Video"},
    {"9H",	"Bottom Up"},
    {"A4",	"Konami (Yu-Gi-Oh!)"},
    {"BL",	"MTO"},
    {"DK",	"Kodansha"},
};
