#ifndef NARATTEMIRU_MIRU_H
#define NARATTEMIRU_MIRU_H

#include <vector>

#include "Naratte.h"
#include "View.h"
#include "views/Menubar.h"
#include "views/AsmView.h"
#include "views/CartridgeInfoView.h"
#include "views/ExportView.h"
#include "views/MemoryView.h"
#include "views/RegisterView.h"
#include "views/ROMView.h"
#include "views/SettingsView.h"
#include "views/TileView.h"
#include "views/ViewportView.h"

class Miru {
public:
    Miru();
    ~Miru();

    void update();

    bool shouldRun() const;

private:
    View* _view{};
    Menubar* _menubar{};
    AsmView* _asm_view{};
    MemoryView* _memory_view{};
    RegisterView* _register_view{};
    ViewportView* _viewport_view{};
    SettingsView* _settings_view{};
    ExportView* _export_view{};
    CartridgeInfoView* _cartridge_info_view{};
    TileView* _tile_view{};
    ROMView* _rom_view{};

    Naratte* _naratte{};

    int _ticks = 1000;

};


#endif //NARATTEMIRU_MIRU_H