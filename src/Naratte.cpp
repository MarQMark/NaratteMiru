#include "Naratte.h"

#include <cstdio>
#include <dlfcn.h>

Naratte::Naratte() {
    reloadLib();
}

Naratte::~Naratte() {
    if (naratte_clean_d)
        naratte_clean_d(_cpu, _ppu, _dasm);
}

void Naratte::reloadLib() {
    if (_lib_handle)
        dlclose(_lib_handle);

    _lib_handle = dlopen("path/libNaratteLib.so", RTLD_LAZY);
    if (!_lib_handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return;
    }

    dlerror();

    naratte_init_d          = reinterpret_cast<int8_t (*)(void**, void**, void**)>(dlsym(_lib_handle, "naratte_init_d"));
    naratte_load_rom        = reinterpret_cast<int8_t (*)(void*, const char*, const char*)>(dlsym(_lib_handle, "naratte_load_rom"));
    naratte_tick            = reinterpret_cast<void   (*)(void*, void*)>(dlsym(_lib_handle, "naratte_tick"));
    naratte_get_ic          = reinterpret_cast<void   (*)(void*, uint8_t*)>(dlsym(_lib_handle, "naratte_get_ic"));
    naratte_disassemble     = reinterpret_cast<char*  (*)(void*, uint8_t*)>(dlsym(_lib_handle, "naratte_disassemble"));
    naratte_disassemble_cpu = reinterpret_cast<char*  (*)(void*, void*)>(dlsym(_lib_handle, "naratte_disassemble_cpu"));
    naratte_clean_d         = reinterpret_cast<void   (*)(void*, void*, void*)>(dlsym(_lib_handle, "naratte_clean_d"));
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

    if(!naratte_load_rom(_cpu,
        "path",
        "path")){
        fprintf(stderr, "Error loading ROMs\n");
        return;
        }

    ((uint8_t*)_cpu)[4137] = 1;
}

void Naratte::update() {
    for(int i = 0; i < 10000; i++) {
        _instructions.emplace_back(Instruction{{0x0, 0x0, 0x0}});
        naratte_get_ic(_cpu, _instructions.back().op);
        naratte_tick(_cpu, _ppu);
    }
}

uint32_t * Naratte::getFB() const {
    return static_cast<uint32_t*>(_ppu);
}

std::vector<Instruction>& Naratte::getInstructions() {
    return _instructions;
}

char* Naratte::getInstructionName(uint8_t *ic) const {
    return naratte_disassemble(_dasm, ic);
}
