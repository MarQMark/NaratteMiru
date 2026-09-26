#include "Naratte.h"

#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>

#include "Config.h"
#include "Util.h"

#include "../disassembler/disassembler.h"

Naratte::Naratte() {
    load_symbols();
    reload();
}

Naratte::~Naratte() {
    if (_builtin_dasm && _dasm)
        dasm_free(reinterpret_cast<dasm **>(&_dasm));

    if (naratte_free_d)
        naratte_free_d(&_cpu, &_mem, &_ppu, &_dasm);
}

void Naratte::reload() {
    Config::get()->Ticks = 1000;
    _selected = 0;
    reloadROM();
    _success = reloadLib();
}

void Naratte::reloadROM() {
    _instructions.clear();
    _mem_writes.clear();
    _call_stack.clear();
    _call_labels.clear();
    load_labels();
}

bool Naratte::reloadLib() {
    _last_time = std::chrono::steady_clock::now();

    _instructions.clear();
    _mem_writes.clear();
    _call_stack.clear();

    if (_builtin_dasm && _dasm)
        dasm_free(reinterpret_cast<dasm **>(&_dasm));

    if (naratte_free_d)
        naratte_free_d(&_cpu, &_mem, &_ppu, &_dasm);
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
    naratte_free_d = nullptr;
    naratte_get_mc = nullptr;
    naratte_init_pseudo_mem = nullptr;
    naratte_free_pseudo_mem = nullptr;
    mem_read = nullptr;
    mem_write = nullptr;
    naratte_get_fb = nullptr;
    naratte_get_fbs = nullptr;

    if (_lib_handle)
        dlclose(_lib_handle);

    if (Util::WaitForStableFile(Config::get()->settings.pathLib.get()))
        _lib_handle = dlopen(Config::get()->settings.pathLib.get().c_str(), RTLD_LAZY);
    if (!_lib_handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return false;
    }

    dlerror();

    naratte_init_d          = reinterpret_cast<int8_t (*)(void**, void**, void**, void**)>(dlsym(_lib_handle, _symbols["Init"].c_str()));
    query_dl_error();
    naratte_load_rom        = reinterpret_cast<int8_t (*)(void*, const char*, const char*)>(dlsym(_lib_handle, _symbols["LoadRom"].c_str()));
    query_dl_error();
    naratte_tick            = reinterpret_cast<void   (*)(void*, void*, void*)>(dlsym(_lib_handle, _symbols["Tick"].c_str()));
    query_dl_error();
    naratte_input           = reinterpret_cast<void   (*)(void*, uint8_t)>(dlsym(_lib_handle, _symbols["Input"].c_str()));
    query_dl_error();
    naratte_get_ic          = reinterpret_cast<void   (*)(void*, uint8_t*)>(dlsym(_lib_handle, _symbols["GetInstructionCache"].c_str()));
    query_dl_error();
    naratte_disassemble     = reinterpret_cast<char*  (*)(void*, uint8_t*)>(dlsym(_lib_handle, _symbols["Disassemble"].c_str()));
    query_dl_error();
    naratte_free_d          = reinterpret_cast<void   (*)(void**, void**, void**, void**)>(dlsym(_lib_handle, _symbols["Free"].c_str()));
    query_dl_error();

    naratte_get_mc          = reinterpret_cast<struct mem_change* (*)(void*)>(dlsym(_lib_handle, _symbols["GetMemoryChange"].c_str()));
    query_dl_error();
    naratte_mc_enable       = reinterpret_cast<void (*)(void*, uint8_t)>(dlsym(_lib_handle, _symbols["EnableMemoryChange"].c_str()));
    query_dl_error();
    naratte_init_pseudo_mem = reinterpret_cast<int8_t (*)(void**, const char*, const char*)>(dlsym(_lib_handle, _symbols["InitPseudoRam"].c_str()));
    query_dl_error();
    naratte_free_pseudo_mem = reinterpret_cast<void   (*)(void**)>(dlsym(_lib_handle, _symbols["FreePseudoRam"].c_str()));
    query_dl_error();
    mem_read                = reinterpret_cast<uint8_t(*)(void*, uint16_t)>(dlsym(_lib_handle, _symbols["MemoryRead"].c_str()));
    query_dl_error();
    mem_write               = reinterpret_cast<void   (*)(void*, uint16_t, uint8_t)>(dlsym(_lib_handle, _symbols["MemoryWrite"].c_str()));
    query_dl_error();

    naratte_get_fb          = reinterpret_cast<uint32_t* (*)(void*)>(dlsym(_lib_handle, _symbols["GetFrameBuffer"].c_str()));
    query_dl_error();
    naratte_get_fbs         = reinterpret_cast<void      (*)(void*, uint32_t**, uint32_t**, uint32_t**, uint32_t**)>(dlsym(_lib_handle, _symbols["GetFrameBuffers"].c_str()));
    query_dl_error();

    naratte_get_snapshot    = reinterpret_cast<void (*)(const void*,const  void*,const void*, uint8_t**, uint32_t*)>(dlsym(_lib_handle, _symbols["GetSnapshot"].c_str()));
    query_dl_error();
    naratte_set_snapshot    = reinterpret_cast<void (*)(void*, void*, void*, const uint8_t*, uint32_t)>(dlsym(_lib_handle, _symbols["SetSnapshot"].c_str()));
    query_dl_error();

    if(!naratte_init_d(&_cpu, &_mem, &_ppu, &_dasm)) {
        fprintf(stderr, "Error init naratte debug\n");
        return false;
    }

    if (naratte_load_rom) {
        if(!naratte_load_rom(
            _mem,
            Config::get()->settings.pathBoot.get().c_str(),
            Config::get()->settings.pathRom.get().c_str()))
            {
            fprintf(stderr, "Error loading ROMs\n");
            return false;
        }
    }

    if (!_dasm) {
        _builtin_dasm = true;
        dasm_init(reinterpret_cast<dasm **>(&_dasm));
    }

    reloadPseudoMem(0);

    return true;
}

void Naratte::update() {
    if(!_success)
        return;

    _dirty = false;

    // Calculate machine expected cycles
    auto now = std::chrono::steady_clock::now();
    double dt = std::chrono::duration<double>(now - _last_time).count();
    // TODO: Fix
    dt = std::min(dt, 1. / 60);//Config::get()->getMinFR());
    _last_time = now;

    double speed = 1;//Config::get()->getSpeedMulti();
    if(mem_read && _mem && (mem_read(_mem, 0xFF4C) & 0x80))
        speed *= 2;

    const auto steps = static_cast<uint32_t>(dt * 4194304 * speed);

    for(int i = 0; i < steps; i++) {
        if (is_inf_loop()) {
            Config::get()->Ticks = 0;
            break;
        }

        if (Config::get()->properties.monitoring.get()) {
            if (naratte_get_ic) {
                _instructions.emplace_back(Instruction{{0xDD, 0xDD, 0xDD, 0x0}});
                naratte_get_ic(_cpu, _instructions.back().op);
                memcpy(&_instructions.back().cpu, _cpu, sizeof(_instructions.back().cpu));
            }
            add_last_call();
        }

        if (naratte_input)
            naratte_input(_cpu, Config::get()->Joypad);
        naratte_tick(_cpu, _mem, _ppu);

        if (Config::get()->properties.monitoring.get() && naratte_get_mc) {
            for (const mem_change* change = naratte_get_mc(_mem); change != nullptr; change = change->next) {
                _mem_writes.emplace_back(MemWrites{_instructions.size() - 1, change->addr, change->data});
            }
        }
    }
}

void Naratte::loadSnapshot() const {
    if(!naratte_set_snapshot)
        return;

    std::ifstream is("./snapshot.nm", std::ios::binary);
    if (!is) {
        printf("Cannot open file: ./snapshot.nm\n");
        return;
    }

    uint32_t size = 0;
    is.read(reinterpret_cast<char*>(&size), sizeof(size));
    void* buffer = malloc(size);
    is.read(static_cast<char*>((buffer)), size);

    if (!is) {
        fprintf(stderr, "Error while reading: ./snapshot.nm\n");
        free(buffer);
        return;
    }

    naratte_set_snapshot(_cpu, _mem, _ppu, static_cast<uint8_t*>(buffer), size);

    free(buffer);
}

void Naratte::saveSnapshot() const {
    if(!naratte_get_snapshot)
        return;

    uint32_t size;
    uint8_t* buffer{};
    naratte_get_snapshot(_cpu, _mem, _ppu, &buffer, &size);

    std::ofstream os("./snapshot.nm", std::ios::binary);
    if (!os) {
        fprintf(stderr, "Cannot open file: ./snapshot.nm\n");
        return;
    }

    os.write(reinterpret_cast<const char*>(&size), sizeof(size));
    os.write(reinterpret_cast<const char*>(buffer), size);

    if (!os) {
        fprintf(stderr, "Error while writing: ./snapshot.nm\n");
    }

    if(buffer)
        free(buffer);
}

uint32_t* Naratte::getFB(const int buffer) const {
    if (!_ppu)
        return nullptr;

    if (buffer == 0) {
        if (!naratte_get_fb)
            return nullptr;

        return naratte_get_fb(_ppu);
    }

    uint32_t *bg{}, *win{}, *obj{}, *prio;
    if (!naratte_get_fbs)
        return nullptr;

    naratte_get_fbs(_ppu, &bg, &win, &obj, &prio);
    if (buffer == 1 && bg)
        return bg;
    if (buffer == 2 && win)
        return win;
    if (buffer == 3 && obj)
        return obj;
    if (buffer == 4 && prio)
        return prio;

    return nullptr;
}

std::vector<Instruction>& Naratte::getInstructions() {
    return _instructions;
}

char* Naratte::getInstructionName(uint8_t *ic) const {
    if(!_success)
        return nullptr;

    if (_builtin_dasm && _dasm) {
        dasm_disassemble(static_cast<dasm*>(_dasm), ic);
        return static_cast<dasm*>(_dasm)->buf;
    }

    if (!naratte_disassemble)
        return nullptr;

    return naratte_disassemble(_dasm, ic);
}

std::vector<MemWrites> & Naratte::getMemWrites() {
    return _mem_writes;
}

void Naratte::reloadPseudoMem(const size_t iId) {
    if(!_success)
        return;

    if (_pseudo_mem)
        naratte_free_pseudo_mem(&_pseudo_mem);

    if (!naratte_init_pseudo_mem)
        return;

    if (!naratte_init_pseudo_mem(
        &_pseudo_mem,
        Config::get()->settings.pathBoot.get().c_str(),
        Config::get()->settings.pathRom.get().c_str()))
        {
        fprintf(stderr, "Error init pseudo ram\n");
        return;
    }

    for (const auto [idx, addr, data] : _mem_writes) {
        if (idx > iId)
            break;

        if (mem_write && _pseudo_mem)
            mem_write(_pseudo_mem, addr, data);
    }

    //((uint8_t*)_pseudo_mem)[98392] = 0;
}

uint8_t Naratte::readPseudoMem(const uint16_t addr) const {
    if(!_success)
        return 0x00;

    if (!mem_read)
        return 0xFF;

    if (_selected < 0)
        return mem_read(_mem, addr);

    if (!_pseudo_mem)
        return 0xFF;

    return mem_read(_pseudo_mem, addr);
}

void Naratte::writePseudoMem(const uint16_t addr, const uint8_t data) const {
    if(!_success)
        return;

    if (!mem_write)
        return;

    if (_selected < 0)
        return mem_write(_mem, addr, data);

    if (!_pseudo_mem)
        return;

    return mem_write(_pseudo_mem, addr, data);
}

void* Naratte::getPseudoMem() const {
    return _pseudo_mem;
}

void Naratte::enableMemChange(const bool enable) const {
    if(!_success)
        return;

    if (!naratte_mc_enable)
        return;

    if (_selected < 0)
        return naratte_mc_enable(_mem, enable ? 1 : 0);

    if (!_pseudo_mem)
        return;

    return naratte_mc_enable(_pseudo_mem, enable ? 1 : 0);
}

void Naratte::setSelected(const int selected) {
    if(!_success)
        return;

    if (selected != _selected) {
        reloadPseudoMem(selected);
        // TODO: fix
        //if (ppu_draw)
        //    ppu_draw(_ppu, _pseudo_mem);
        _dirty = true;
    }

    _selected = selected;
}

int Naratte::getSelected() const {
    if(!_success)
        return -1;

    return _selected;
}

bool Naratte::isDirty() const {
    return _dirty;
}

void Naratte::resetDirty() {
    _dirty = false;
}

std::string Naratte::getCallLabel(const uint16_t addr) {
    if (_call_labels.contains(addr))
        return _call_labels[addr];

    return "na";
}

std::vector<std::pair<int, int>> & Naratte::getCallStack() {
    return _call_stack;
}

void Naratte::serialize(const std::string &path) const {
    std::ofstream os(path, std::ios::binary);
    if (!os) {
        fprintf(stderr, "Cannot open file: %s\n", path.c_str());
        return;
    }

    const uint64_t ins_count = _instructions.size();
    os.write(reinterpret_cast<const char*>(&ins_count), sizeof(ins_count));

    for (const auto &[op, cpu] : _instructions) {
        os.write(reinterpret_cast<const char*>(op), sizeof(op));
        os.write(reinterpret_cast<const char*>(&cpu), sizeof(cpu));
    }

    const uint64_t mw_count = _mem_writes.size();
    os.write(reinterpret_cast<const char*>(&mw_count), sizeof(mw_count));

    for (const auto &[idx, addr, data] : _mem_writes) {
        os.write(reinterpret_cast<const char*>(&idx),  sizeof(idx));
        os.write(reinterpret_cast<const char*>(&addr), sizeof(addr));
        os.write(reinterpret_cast<const char*>(&data), sizeof(data));
    }

    if (!os) {
        fprintf(stderr, "Error while writing: %s\n", path.c_str());
        return;
    }
}

void Naratte::deserialize(const std::string &path) {
    _instructions.clear();
    _mem_writes.clear();
    _call_stack.clear();
    _call_labels.clear();
    _selected = -1;
    load_labels();
    Config::get()->Ticks = 0;
    Config::get()->Pause = true;

    std::ifstream is(path, std::ios::binary);
    if (!is) {
        fprintf(stderr, "Cannot open file: %s\n", path.c_str());
        return;
    }

    uint64_t ins_count = 0;
    is.read(reinterpret_cast<char*>(&ins_count), sizeof(ins_count));
    _instructions.resize(ins_count);

    for (uint64_t i = 0; i < ins_count; ++i) {
        auto &[op, cpu] = _instructions[i];
        is.read(reinterpret_cast<char*>(op), sizeof(op));
        is.read(reinterpret_cast<char*>(&cpu), sizeof(cpu));
    }

    uint64_t mw_count = 0;
    is.read(reinterpret_cast<char*>(&mw_count), sizeof(mw_count));
    _mem_writes.resize(mw_count);

    for (uint64_t i = 0; i < mw_count; ++i) {
        auto &mw = _mem_writes[i];
        is.read(reinterpret_cast<char*>(&mw.idx),  sizeof(mw.idx));
        is.read(reinterpret_cast<char*>(&mw.addr), sizeof(mw.addr));
        is.read(reinterpret_cast<char*>(&mw.data), sizeof(mw.data));
    }

    if (!is) {
        fprintf(stderr, "Error while reading: %s\n", path.c_str());
        return;
    }
}

bool Naratte::is_inf_loop() const {
    if (!Config::get()->settings.stopInfLoop.get())
        return false;

    // JR -2
    if (!_instructions.empty() &&
        _instructions.back().op[0] == 0x18 &&
        _instructions.back().op[1] == 0xFE)
        return true;

    // JR -3, NOP
    if (_instructions.size() >= 2 &&
        _instructions[_instructions.size() - 2].op[0] == 0x18 &&
        _instructions[_instructions.size() - 2].op[1] == 0xFD &&
        _instructions.back().op[0] == 0x00)
        return true;

    return false;
}

void Naratte::load_labels() {
    std::filesystem::path p = Config::get()->settings.pathRom.get();
    p.replace_extension(".sym");
    std::string symPath = p.string();

    std::ifstream file(symPath);
    if (!file.is_open()) {
        return;
    }

    std::string line;
    bool inLabels = false;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == ';')
            continue;
        if (line == "[labels]") {
            inLabels = true;
            continue;
        }
        if (line.size() > 0 && line[0] == '[' && inLabels && line != "[labels]") {
            break;
        }
        if (!inLabels)
            continue;

        std::istringstream iss(line);

        std::string addrStr;
        std::string name;
        if (!(iss >> addrStr >> name))
            continue;

        auto colonPos = addrStr.find(':');
        if (colonPos == std::string::npos)
            continue;

        std::string bankStr  = addrStr.substr(0, colonPos);
        std::string offStr   = addrStr.substr(colonPos + 1);

        try {
            _call_labels[static_cast<uint16_t>(std::stoul(offStr, nullptr, 16))]  = name;
        } catch (...) {
        }
    }
}

void Naratte::load_symbols() {
    _symbols["Init"] = "naratte_init_d";
    _symbols["LoadRom"] = "naratte_load_rom";
    _symbols["Tick"] = "naratte_tick";
    _symbols["Input"] = "naratte_input";
    _symbols["GetInstructionCache"] = "naratte_get_ic";
    _symbols["Disassemble"] = "naratte_disassemble";
    _symbols["Free"] = "naratte_free_d";
    _symbols["GetMemoryChange"] = "naratte_get_mc";
    _symbols["EnableMemoryChange"] = "naratte_mc_enable";
    _symbols["InitPseudoRam"] = "naratte_init_pseudo_mem";
    _symbols["FreePseudoRam"] = "naratte_free_pseudo_mem";
    _symbols["MemoryRead"] = "mem_read";
    _symbols["MemoryWrite"] = "mem_write";
    _symbols["GetFrameBuffer"] = "naratte_get_fb";
    _symbols["GetFrameBuffers"] = "naratte_get_fbs";
    _symbols["GetSnapshot"] = "naratte_get_snapshot";
    _symbols["SetSnapshot"] = "naratte_set_snapshot";

    const std::string path = "symbols.conf";
    if (!std::filesystem::exists(path))
        return;
    std::ifstream file(path);
    if (!file.is_open())
        return;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;
        const auto pos = line.find(':');
        if (pos == std::string::npos)
            continue;

        std::string key  = line.substr(0, pos);
        std::string name = line.substr(pos + 1);

        auto trim = [](std::string& s) {
            const auto ws = " \t\n\r";
            s.erase(0, s.find_first_not_of(ws));
            s.erase(s.find_last_not_of(ws) + 1);
        };

        trim(key);
        trim(name);

        if (!key.empty() && !name.empty())
            _symbols[key] = name;
    }
}

void Naratte::query_dl_error() {
    if (const char* err = dlerror()) {
        fprintf(stderr, "Symbol error: %s\n", err);
    }
}

void Naratte::add_last_call() {
    // TODO: Fix
    if (const auto instruction = _instructions.back(); instruction.isCall() ||
        (instruction.isJP() /*&& Config::get()->isJPasCall(_instructions.size() - 1, _instructions.size() - 1)*/))
        _call_stack.emplace_back(std::pair<int, int>{_instructions.size() - 1, CALL});
    else if (instruction.isRet())
        _call_stack.emplace_back(std::pair<int, int>{_instructions.size() - 1, RET});
}

void Naratte::rebuildCallStack() {
    _call_stack.clear();
    // TODO: Fix
    for (int i = 0; i < _instructions.size(); i++) {
        if (const auto& instruction = _instructions[i]; instruction.isCall() ||
           (instruction.isJP() /*&& Config::get()->isJPasCall(i, _instructions.size() - 1)*/))
            _call_stack.emplace_back(std::pair<int, int>{i, CALL});
        else if (instruction.isRet())
            _call_stack.emplace_back(std::pair<int, int>{i, RET});
    }
}
