#include "Naratte.h"

#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>

#include "Config.h"
#include "Util.h"

Naratte::Naratte() {
    load_symbols();
    reload();
}

Naratte::~Naratte() {
    if (naratte_free_d)
        naratte_free_d(&_cpu, &_ppu, &_dasm);
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
    _instructions.clear();
    _mem_writes.clear();
    _call_stack.clear();

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
    naratte_free_d = nullptr;
    naratte_get_mc = nullptr;
    naratte_init_pseudo_mem = nullptr;
    naratte_free_pseudo_mem = nullptr;
    mem_read = nullptr;
    mem_write = nullptr;
    ppu_draw = nullptr;

    if (_lib_handle)
        dlclose(_lib_handle);

    if (Util::WaitForStableFile(Config::get()->getLibPath()))
        _lib_handle = dlopen(Config::get()->getLibPath().c_str(), RTLD_LAZY);
    if (!_lib_handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return false;
    }

    dlerror();

    naratte_init_d          = reinterpret_cast<int8_t (*)(void**, void**, void**)>(dlsym(_lib_handle, _symbols["Init"].c_str()));
    if (query_dl_error()) return false;
    naratte_load_rom        = reinterpret_cast<int8_t (*)(void*, const char*, const char*)>(dlsym(_lib_handle, _symbols["LoadRom"].c_str()));
    if (query_dl_error()) return false;
    naratte_tick            = reinterpret_cast<void   (*)(void*, void*)>(dlsym(_lib_handle, _symbols["Tick"].c_str()));
    if (query_dl_error()) return false;
    naratte_get_ic          = reinterpret_cast<void   (*)(void*, uint8_t*)>(dlsym(_lib_handle, _symbols["GetInstructionCache"].c_str()));
    if (query_dl_error()) return false;
    naratte_disassemble     = reinterpret_cast<char*  (*)(void*, uint8_t*)>(dlsym(_lib_handle, _symbols["Disassemble"].c_str()));
    if (query_dl_error()) return false;
    naratte_free_d          = reinterpret_cast<void   (*)(void**, void**, void**)>(dlsym(_lib_handle, _symbols["Free"].c_str()));
    if (query_dl_error()) return false;

    naratte_get_mc          = reinterpret_cast<struct mem_change* (*)(void*)>(dlsym(_lib_handle, _symbols["GetMemoryChange"].c_str()));
    if (query_dl_error()) return false;
    naratte_init_pseudo_mem = reinterpret_cast<int8_t (*)(void**, const char*, const char*)>(dlsym(_lib_handle, _symbols["InitPseudoRam"].c_str()));
    if (query_dl_error()) return false;
    naratte_free_pseudo_mem = reinterpret_cast<void   (*)(void**)>(dlsym(_lib_handle, _symbols["FreePseudoRam"].c_str()));
    if (query_dl_error()) return false;
    mem_read                = reinterpret_cast<uint8_t(*)(void*, uint16_t)>(dlsym(_lib_handle, _symbols["MemoryRead"].c_str()));
    if (query_dl_error()) return false;
    mem_write               = reinterpret_cast<void   (*)(void*, uint16_t, uint8_t)>(dlsym(_lib_handle, _symbols["MemoryWrite"].c_str()));
    if (query_dl_error()) return false;

    ppu_draw                = reinterpret_cast<void   (*)(void*, void*)>(dlsym(_lib_handle, _symbols["PPUDraw"].c_str()));
    if (query_dl_error()) return false;

    if(!naratte_init_d(&_cpu, &_ppu, &_dasm)) {
        fprintf(stderr, "Error init naratte debug\n");
        return false;
    }

    if(!naratte_load_rom(_cpu, Config::get()->getBootPath().c_str(), Config::get()->getGamePath().c_str())) {
        fprintf(stderr, "Error loading ROMs\n");
        return false;
    }

    reloadPseudoMem(0);

    ((uint8_t*)_cpu)[4137] = 1;

    return true;
}

void Naratte::update() {
    if(!_success)
        return;

    _dirty = false;

    for(int i = 0; i < 10000; i++) {
        if (is_inf_loop()) {
            Config::get()->Ticks = 0;
            break;
        }

        if (Config::get()->isMonitored()) {
            _instructions.emplace_back(Instruction{{0xDD, 0xDD, 0xDD, 0x0}});
            naratte_get_ic(_cpu, _instructions.back().op);
            memcpy(&_instructions.back().cpu, _cpu, sizeof(_instructions.back().cpu));
            add_last_call();
        }

        naratte_tick(_cpu, _ppu);

        if (Config::get()->isMonitored()) {
            for (const mem_change* change = naratte_get_mc(_cpu); change != nullptr; change = change->next) {
                _mem_writes.emplace_back(MemWrites{_instructions.size() - 1, change->addr, change->data});
            }
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
    if(!_success)
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

    if (!naratte_init_pseudo_mem(&_pseudo_mem, Config::get()->getBootPath().c_str(), Config::get()->getGamePath().c_str())) {
        fprintf(stderr, "Error init pseudo ram\n");
        return;
    }

    for (const auto [idx, addr, data] : _mem_writes) {
        if (idx > iId)
            break;

        mem_write(_pseudo_mem, addr, data);
    }

    //((uint8_t*)_pseudo_mem)[98392] = 0;
}

uint8_t Naratte::readPseudoMem(const uint16_t addr) const {
    if(!_success)
        return 0x00;

    return mem_read(_pseudo_mem, addr);
}

void* Naratte::getPseudoMem() const {
    return _pseudo_mem;
}

void Naratte::setSelected(const int selected) {
    if(!_success)
        return;

    if (selected != _selected) {
        reloadPseudoMem(selected);
        ppu_draw(_ppu, _pseudo_mem);
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
    std::filesystem::path p = Config::get()->getGamePath();
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
    _symbols["GetInstructionCache"] = "naratte_get_ic";
    _symbols["Disassemble"] = "naratte_disassemble";
    _symbols["Free"] = "naratte_free_d";
    _symbols["GetMemoryChange"] = "naratte_get_mc";
    _symbols["InitPseudoRam"] = "naratte_init_pseudo_mem";
    _symbols["FreePseudoRam"] = "naratte_free_pseudo_mem";
    _symbols["MemoryRead"] = "mem_read";
    _symbols["MemoryWrite"] = "mem_write";
    _symbols["PPUDraw"] = "ppu_draw";

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

bool Naratte::query_dl_error() const {
    if (const char* err = dlerror()) {
        fprintf(stderr, "Symbol error: %s\n", err);
        dlclose(_lib_handle);
        return true;
    }

    return false;
}

void Naratte::add_last_call() {
    if (const auto instruction = _instructions.back(); instruction.isCall() ||
        (instruction.isJP() &&  Config::get()->isJPasCall(_instructions.size() - 1, _instructions.size() - 1)))
        _call_stack.emplace_back(std::pair<int, int>{_instructions.size() - 1, CALL});
    else if (instruction.isRet())
        _call_stack.emplace_back(std::pair<int, int>{_instructions.size() - 1, RET});
}

void Naratte::rebuildCallStack() {
    _call_stack.clear();
    for (int i = 0; i < _instructions.size(); i++) {
        if (const auto& instruction = _instructions[i]; instruction.isCall() ||
           (instruction.isJP() &&  Config::get()->isJPasCall(i, _instructions.size() - 1)))
            _call_stack.emplace_back(std::pair<int, int>{i, CALL});
        else if (instruction.isRet())
            _call_stack.emplace_back(std::pair<int, int>{i, RET});
    }
}
