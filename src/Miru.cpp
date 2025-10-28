#include "Miru.h"

#include <dlfcn.h>

Miru::Miru() {
    _view = new View;
    _menubar = new Menubar;
    _asm_view = new AsmView;
    _memory_view = new MemoryView;
    _register_view = new RegisterView;
    _viewport_view = new ViewportView;

    _view->addViewable(_menubar, "Menubar");
    _view->addViewable(_asm_view, "Assembly");
    _view->addViewable(_memory_view, "Memory");
    _view->addViewable(_register_view, "Registers");
    _view->addViewable(_viewport_view, "Viewport");

    reload_lib();
}

Miru::~Miru() {
    delete _view;
    delete _menubar;
    delete _asm_view;
    delete _memory_view;
    delete _register_view;
    delete _viewport_view;

    if (_lib_handle)
        dlclose(_lib_handle);
}

void Miru::update() {
    if (_menubar->reload()) {
        _menubar->resetReload();
        reload_lib();
    }

    if(_ticks > 0) {
        naratte_tick(_cpu, _ppu);
        printf("%s\n", naratte_disassemble(_cpu, _dasm));
        _ticks--;
    }

    _view->render();
}

bool Miru::shouldRun() const {
    return _view->shouldRun();
}

void Miru::reload_lib() {
    if (_lib_handle)
        dlclose(_lib_handle);

    _lib_handle = dlopen("lib_path.so", RTLD_LAZY);
    if (!_lib_handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return;
    }

    dlerror();

    naratte_init_d = reinterpret_cast<int8_t (*)(void**, void**, void**)>(dlsym(_lib_handle, "naratte_init_d"));
    naratte_load_rom = reinterpret_cast<int8_t (*)(void*, const char*, const char*)>(dlsym(_lib_handle, "naratte_load_rom"));
    naratte_tick = reinterpret_cast<void (*)(void*, void*)>(dlsym(_lib_handle, "naratte_tick"));
    naratte_disassemble = reinterpret_cast<char* (*)(void*, void*)>(dlsym(_lib_handle, "naratte_disassemble"));
    naratte_clean_d = reinterpret_cast<void (*)(void*, void*, void*)>(dlsym(_lib_handle, "naratte_clean_d"));
    const char* err = dlerror();
    if (err) {
        fprintf(stderr, "Symbol error: %s\n", err);
        dlclose(_lib_handle);
        return;
    }

    if(!naratte_init_d(&_cpu, &_ppu, &_dasm)) {
        fprintf(stderr, "Error init naratte debug\n");
        return;
    }

    if(!naratte_load_rom(_cpu, "/home/nan/Downloads/cgb_boot.bin", "/home/nan/Downloads/mts-20240926-1737-443f6e1/acceptance/ppu/stat_lyc_onoff.gb")){
        fprintf(stderr, "Error loading ROMs\n");
        return;
    }
}
