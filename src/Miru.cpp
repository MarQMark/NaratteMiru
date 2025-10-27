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
    }

    printf("test\n");

    dlerror();

    //dasm_init = reinterpret_cast<void (*)(void**)>(dlsym(_lib_handle, "dasm_init"));
    //dasm_disassemble = reinterpret_cast<void (*)(void*, uint8_t*, uint16_t)>(dlsym(_lib_handle, "dasm_disassemble"));
    const char* err = dlerror();
    if (err) {
        fprintf(stderr, "Symbol error: %s\n", err);
        dlclose(_lib_handle);
    }
}
