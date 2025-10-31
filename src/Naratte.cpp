#include "Naratte.h"

#include <cstdio>
#include <cstring>
#include <dlfcn.h>

Naratte::Naratte() {
    _lib_path  = "";
    _boot_path = "";
    _game_path = "";
    reloadLib();
}

Naratte::~Naratte() {
    if (naratte_free_d)
        naratte_free_d(&_cpu, &_ppu, &_dasm);
}

void Naratte::reloadLib() {
    _instructions.clear();
    _mem_writes.clear();

    if (naratte_free_d)
        naratte_free_d(&_cpu, &_ppu, &_dasm);
    if (naratte_free_pseudo_mem && _pseudo_mem)
        naratte_free_pseudo_mem(&_pseudo_mem);

    _cpu = nullptr;
    _ppu = nullptr;
    _dasm = nullptr;
    _pseudo_mem = nullptr;
    naratte_init_d = nullptr;
    naratte_load_rom = nullptr;
    naratte_tick = nullptr;
    naratte_get_ic = nullptr;
    naratte_disassemble = nullptr;
    naratte_disassemble_cpu = nullptr;
    naratte_free_d = nullptr;
    naratte_get_mc = nullptr;
    naratte_init_pseudo_mem = nullptr;
    naratte_free_pseudo_mem = nullptr;
    mem_read = nullptr;
    mem_write = nullptr;
    ppu_draw = nullptr;

    if (_lib_handle)
        dlclose(_lib_handle);

    _lib_handle = dlopen(_lib_path.c_str(), RTLD_LAZY);
    if (!_lib_handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return;
    }

    dlerror();

    naratte_init_d          = reinterpret_cast<int8_t (*)(void**, void**, void**)>(dlsym(_lib_handle, "naratte_init_d"));
    if (query_dl_error()) return;
    naratte_load_rom        = reinterpret_cast<int8_t (*)(void*, const char*, const char*)>(dlsym(_lib_handle, "naratte_load_rom"));
    if (query_dl_error()) return;
    naratte_tick            = reinterpret_cast<void   (*)(void*, void*)>(dlsym(_lib_handle, "naratte_tick"));
    if (query_dl_error()) return;
    naratte_get_ic          = reinterpret_cast<void   (*)(void*, uint8_t*)>(dlsym(_lib_handle, "naratte_get_ic"));
    if (query_dl_error()) return;
    naratte_disassemble     = reinterpret_cast<char*  (*)(void*, uint8_t*, uint8_t*)>(dlsym(_lib_handle, "naratte_disassemble"));
    if (query_dl_error()) return;
    naratte_disassemble_cpu = reinterpret_cast<char*  (*)(void*, void*)>(dlsym(_lib_handle, "naratte_disassemble_cpu"));
    if (query_dl_error()) return;
    naratte_free_d          = reinterpret_cast<void   (*)(void**, void**, void**)>(dlsym(_lib_handle, "naratte_free_d"));
    if (query_dl_error()) return;

    naratte_get_mc          = reinterpret_cast<struct mem_change* (*)(void*)>(dlsym(_lib_handle, "naratte_get_mc"));
    if (query_dl_error()) return;
    naratte_init_pseudo_mem = reinterpret_cast<int8_t (*)(void**, const char*, const char*)>(dlsym(_lib_handle, "naratte_init_pseudo_mem"));
    if (query_dl_error()) return;
    naratte_free_pseudo_mem = reinterpret_cast<void   (*)(void**)>(dlsym(_lib_handle, "naratte_free_pseudo_mem"));
    if (query_dl_error()) return;
    mem_read                = reinterpret_cast<uint8_t(*)(void*, uint16_t)>(dlsym(_lib_handle, "mem_read"));
    if (query_dl_error()) return;
    mem_write               = reinterpret_cast<void   (*)(void*, uint16_t, uint8_t)>(dlsym(_lib_handle, "mem_write"));
    if (query_dl_error()) return;

    ppu_draw                = reinterpret_cast<void   (*)(void*, void*)>(dlsym(_lib_handle, "ppu_draw"));
    if (query_dl_error()) return;

    if(!naratte_init_d(&_cpu, &_ppu, &_dasm)) {
        fprintf(stderr, "Error init naratte debug\n");
        return;
    }

    if(!naratte_load_rom(_cpu, _boot_path.c_str(), _game_path.c_str())) {
        fprintf(stderr, "Error loading ROMs\n");
        return;
    }

    reloadPseudoMem(0);

    ((uint8_t*)_cpu)[4137] = 1;
}

void Naratte::update() {
    _dirty = false;

    for(int i = 0; i < 10000; i++) {
        _instructions.emplace_back(Instruction{{0x0, 0x0, 0x0, 0x1}});
        naratte_get_ic(_cpu, _instructions.back().op);
        memcpy(&_instructions.back().cpu, _cpu, sizeof(_instructions.back().cpu));
        naratte_tick(_cpu, _ppu);

        for (const mem_change* change = naratte_get_mc(_cpu); change != nullptr; change = change->next) {
            _mem_writes.emplace_back(MemWrites{_instructions.size() - 1, change->addr, change->data});
        }
    }
}

uint32_t * Naratte::getFB() const {
    return static_cast<uint32_t*>(_ppu);
}

std::vector<Instruction>& Naratte::getInstructions() {
    return _instructions;
}

char* Naratte::getInstructionName(uint8_t *ic) const {
    return naratte_disassemble(_dasm, ic, &ic[3]);
}

std::vector<MemWrites> & Naratte::getMemWrites() {
    return _mem_writes;
}

void Naratte::reloadPseudoMem(const size_t iId) {
    if (_pseudo_mem)
        naratte_free_pseudo_mem(&_pseudo_mem);

    if (!naratte_init_pseudo_mem(&_pseudo_mem, _boot_path.c_str(), _game_path.c_str())) {
        fprintf(stderr, "Error init pseudo ram\n");
        return;
    }

    for (const auto [idx, addr, data] : _mem_writes) {
        if (idx > iId)
            break;

        mem_write(_pseudo_mem, addr, data);
    }

    ((uint8_t*)_pseudo_mem)[98392] = 0;
}

uint8_t Naratte::readPseudoMem(const uint16_t addr) const {
    return mem_read(_pseudo_mem, addr);
}

void* Naratte::getPseudoMem() const {
    return _pseudo_mem;
}

void Naratte::setSelected(const int selected) {
    if (selected != _selected) {
        reloadPseudoMem(selected);
        ppu_draw(_ppu, _pseudo_mem);
        _dirty = true;
    }

    _selected = selected;
}

int Naratte::getSelected() const {
    return _selected;
}

bool Naratte::isDirty() const {
    return _dirty;
}

void Naratte::resetDirty() {
    _dirty = false;
}

bool Naratte::query_dl_error() const {
    if (const char* err = dlerror()) {
        fprintf(stderr, "Symbol error: %s\n", err);
        dlclose(_lib_handle);
        return true;
    }

    return false;
}
