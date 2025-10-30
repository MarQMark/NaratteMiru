#include "Miru.h"

#include <dlfcn.h>

Miru::Miru() {
    _naratte = new Naratte;

    _view = new View;
    _menubar = new Menubar;
    _asm_view = new AsmView(_naratte);
    _memory_view = new MemoryView(_naratte);
    _register_view = new RegisterView(_naratte);
    _viewport_view = new ViewportView;

    _view->addViewable(_menubar, "Menubar");
    _view->addViewable(_asm_view, "Assembly");
    _view->addViewable(_memory_view, "Memory");
    _view->addViewable(_register_view, "Registers");
    _view->addViewable(_viewport_view, "Viewport");
}

Miru::~Miru() {
    delete _view;
    delete _menubar;
    delete _asm_view;
    delete _memory_view;
    delete _register_view;
    delete _viewport_view;
    delete _naratte;
}

void Miru::update() {
    if (_menubar->reload()) {
        _menubar->resetReload();
        _naratte->reloadLib();
        _ticks = 1000;
    }

    if(_ticks > 0) {
        _naratte->update();
        _viewport_view->updateFB(_naratte->getFB());
        _ticks--;
    }

    _view->render();
}

bool Miru::shouldRun() const {
    return _view->shouldRun();
}