#ifndef NARATTEMIRU_MIRU_H
#define NARATTEMIRU_MIRU_H

#include "View.h"
#include "views/Menubar.h"
#include "views/AsmView.h"
#include "views/MemoryView.h"
#include "views/RegisterView.h"
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

    void* _lib_handle{};
    void reload_lib();


    void* _cpu{};
    void* _ppu{};
    void* _dasm{};
    //int8_t (*naratte_init)(void** cpu, void** ppu){};
    int8_t (*naratte_init_d)(void** cpu, void** ppu, void** dasm){};

    int8_t (*naratte_load_rom)(void* cpu, const char* boot, const char* game);

    void (*naratte_tick)(void* cpu, void* ppu){};
    char* (*naratte_disassemble)(void* cpu, void* dasm){};

    //void (*naratte_clean)(void* cpu, void* ppu){};
    void (*naratte_clean_d)(void* cpu, void* ppu, void* dasm){};

    int _ticks = 1000;
};


#endif //NARATTEMIRU_MIRU_H