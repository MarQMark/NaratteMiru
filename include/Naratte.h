#ifndef NARATTEMIRU_NARATTE_H
#define NARATTEMIRU_NARATTE_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct Instruction {
    uint8_t op[4] = {0xDD, 0xDD, 0xDD, 0x00}; // Invalid Opcodes
    struct sm83 {
        uint8_t IME;
        union {
            uint16_t AF;
            struct {
                uint8_t F;  // Flags
                uint8_t A;  // Accumulator

#define FLAG_Z 0b10000000 // Zero
#define FLAG_N 0b01000000 // Subtract
#define FLAG_H 0b00100000 // Half-Carry
#define FLAG_C 0b00010000 // Carry
            };
        };

        union {
            uint16_t BC;
            struct {
                uint8_t C;
                uint8_t B;
            };
        };
        union {
            uint16_t DE;
            struct {
                uint8_t E;
                uint8_t D;
            };
        };
        union {
            uint16_t HL;
            struct {
                uint8_t L;
                uint8_t H;
            };
        };

        uint16_t PC; // Program Counter
        uint16_t SP; // Stack Pointer
    } cpu;

    enum Type {
        UDEF = 0,
        LOAD_8 = 1,
        LOAD_16 = 2,
        ARI_8 = 3,
        ARI_16 = 4,
        BIT = 5,
        FLOW = 6,
        MISC = 7
    };
private:
    static const uint8_t type[256];
public:
    uint8_t getType() const {
        return type[op[0]];
    }

    bool isCall() const {
        switch (op[0]) {
            case 0xCD: return true;                  // CALL nn
            case 0xC4: return (cpu.F & FLAG_Z) == 0; // CALL NZ, nn
            case 0xD4: return (cpu.F & FLAG_C) == 0; // CALL NC, nn
            case 0xCC: return (cpu.F & FLAG_Z);      // CALL  Z, nn
            case 0xDC: return (cpu.F & FLAG_C);      // CALL  C, nn
            case 0xC7: return true;                  // RST 0x00
            case 0xD7: return true;                  // RST 0x10
            case 0xE7: return true;                  // RST 0x20
            case 0xF7: return true;                  // RST 0x40
            case 0xCF: return true;                  // RST 0x08
            case 0xDF: return true;                  // RST 0x18
            case 0xEF: return true;                  // RST 0x28
            case 0xFF: return true;                  // RST 0x38
            default: return false;
        }
    }
    bool isJP() const {
        switch (op[0]) {
            case 0xC3: return true;                  // JP nn
            case 0xE9: return true;                  // JP HL
            case 0xC2: return (cpu.F & FLAG_Z) == 0; // JP NZ, nn
            case 0xD2: return (cpu.F & FLAG_C) == 0; // JP NC, nn
            case 0xCA: return (cpu.F & FLAG_Z);      // JP  Z, nn
            case 0xDA: return (cpu.F & FLAG_C);      // JP  C, nn
            default: return false;
        }
    }
    bool isRet() const {
        switch (op[0]) {
            case 0xC9: return true;                  // RET
            case 0xD9: return true;                  // RETI
            case 0xC0: return (cpu.F & FLAG_Z) == 0; // RET NZ
            case 0xD0: return (cpu.F & FLAG_C) == 0; // RET NC
            case 0xC8: return (cpu.F & FLAG_Z);      // RET  Z
            case 0xD8: return (cpu.F & FLAG_C);      // RET  C
            default: return false;
        }
    }
};
struct MemWrites {
    size_t idx = 0;
    uint16_t addr = 0;
    uint8_t  data = 0;
};

class Naratte {
public:
    Naratte();
    ~Naratte();

    void reload();
    void reloadROM();
    bool reloadLib();
    void update();

    void rebuildCallStack();

    uint32_t* getFB() const;
    std::vector<Instruction>& getInstructions();
    char* getInstructionName(uint8_t* ic) const;

    std::vector<MemWrites>& getMemWrites();

    void reloadPseudoMem(size_t iId);
    uint8_t readPseudoMem(uint16_t addr) const;
    void writePseudoMem(uint16_t addr, uint8_t data) const;
    void* getPseudoMem() const;
    void enableMemChange(bool enable) const;

    void setSelected(int selected);
    int getSelected() const;

    bool isDirty() const;
    void resetDirty();

    std::string getCallLabel(uint16_t addr);

    enum {
        CALL,
        RET
    };
    std::vector<std::pair<int, int>>& getCallStack();

    void serialize(const std::string& path) const;
    void deserialize(const std::string& path);

private:
    bool _success = false;

    int _selected = -1;
    bool _dirty = false;

    bool is_inf_loop() const;

    std::map<uint16_t, std::string> _call_labels;
    void load_labels();

    std::map<std::string, std::string> _symbols;
    void load_symbols();
    void* _lib_handle{};

    bool query_dl_error() const;

    void* _cpu{};
    void* _mem{};
    void* _ppu{};
    void* _dasm{};
    void* _pseudo_mem{};

    int8_t (*naratte_init_d)(void** cpu, void** mem, void** ppu, void** dasm){};
    int8_t (*naratte_load_rom)(void* mem, const char* boot, const char* game){};

    void (*naratte_tick)(void* cpu, void* mem, void* ppu){};
    void (*naratte_input)(void* cpu, uint8_t);

    void (*naratte_get_ic)(void *cpu, uint8_t* ic){};
    char* (*naratte_disassemble)(void* dasm, uint8_t *ins){};

    void (*naratte_free_d)(void** cpu, void** mem, void** ppu, void** dasm){};

    struct mem_change {
        struct mem_change* next;
        uint16_t addr;
        uint8_t data;
    };
    struct mem_change* (*naratte_get_mc)(void* mem){};
    void (*naratte_mc_enable)(void* mem, uint8_t enable){};
    int8_t (*naratte_init_pseudo_mem)(void** mem, const char* boot, const char* game){};
    void (*naratte_free_pseudo_mem)(void** mem){};
    uint8_t (*mem_read)(void* mem, uint16_t addr){};
    void (*mem_write)(void* mem, uint16_t addr, uint8_t data){};

    std::vector<Instruction> _instructions;
    std::vector<MemWrites> _mem_writes;

    std::vector<std::pair<int, int>> _call_stack;
    void add_last_call();
};


#endif //NARATTEMIRU_NARATTE_H