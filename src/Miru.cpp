#include "Miru.h"

#include <Config.h>
#include <dlfcn.h>
#include <imgui.h>

Miru::Miru() {
    _naratte = new Naratte;

    _view = new View;
    _menubar = new Menubar(_naratte);
    _asm_view = new AsmView(_naratte);
    _memory_view = new MemoryView(_naratte);
    _register_view = new RegisterView(_naratte);
    _viewport_view = new ViewportView(_naratte);
    _settings_view = new SettingsView();
    _export_view = new ExportView(_naratte);
    _cartridge_info_view = new CartridgeInfoView(_naratte);
    _tile_view = new TileView(_naratte);
    _rom_view = new ROMView(_naratte);

    _view->addViewable(_menubar, "Menubar");
    _view->addViewable(_asm_view, "Assembly");
    _view->addViewable(_memory_view, "Memory");
    _view->addViewable(_register_view, "Registers");
    _view->addViewable(_viewport_view, "Viewport");
    _view->addViewable(_settings_view, "Settings");
    _view->addViewable(_export_view, "Export");
    _view->addViewable(_cartridge_info_view, "Cartridge Info");
    _view->addViewable(_tile_view, "Tiles");
    _view->addViewable(_rom_view, "ROMs");
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
    if (Config::get()->Reload || Config::get()->libNaratteChanged()) {
        Config::get()->load();
        _naratte->reload();

        Config::get()->Reload = false;
    }

    if (Config::get()->dirtyCallStack())
        _naratte->rebuildCallStack();

    if(Config::get()->Ticks > 0 && !Config::get()->Pause) {
        _naratte->update();
    }

    _viewport_view->updateFB(_naratte->getFB(_viewport_view->getSelectedBuffer()));

    if (ImGui::GetIO().KeyCtrl) {
        if(ImGui::IsKeyPressed(ImGuiKey_R))
            _naratte->reload();

        if(ImGui::IsKeyPressed(ImGuiKey_E))
            _naratte->saveSnapshot();
        if(ImGui::IsKeyPressed(ImGuiKey_R))
            _naratte->loadSnapshot();
    }

    _view->render();
}

bool Miru::shouldRun() const {
    return _view->shouldRun();
}