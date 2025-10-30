#ifndef NARATTEMIRU_NARATTE_H
#define NARATTEMIRU_NARATTE_H

#include <cstdint>
#include <string>
#include <vector>

struct Instruction {
    uint8_t op[4] = {0xFD, 0xFD, 0xFD}; // Invalid Opcodes
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

    void reloadLib();
    void update();

    uint32_t* getFB() const;
    std::vector<Instruction>& getInstructions();
    char* getInstructionName(uint8_t* ic) const;

    std::vector<MemWrites>& getMemWrites();

    void reloadPseudoMem(size_t iId);
    uint8_t readPseudoMem(uint16_t addr) const;
    void* getPseudoMem() const;

private:
    std::string _lib_path;
    std::string _boot_path;
    std::string _game_path;

    void* _lib_handle{};

    void* _cpu{};
    void* _ppu{};
    void* _dasm{};
    void* _pseudo_mem{};

    //int8_t (*naratte_init)(void** cpu, void** ppu){};
    int8_t (*naratte_init_d)(void** cpu, void** ppu, void** dasm){};
    int8_t (*naratte_load_rom)(void* cpu, const char* boot, const char* game){};

    void (*naratte_tick)(void* cpu, void* ppu){};

    void (*naratte_get_ic)(void *cpu, uint8_t* ic){};
    char* (*naratte_disassemble)(void* dasm, uint8_t *ins, uint8_t* mcc){};
    char* (*naratte_disassemble_cpu)(void* cpu, void* dasm){};

    //void (*naratte_clean)(void* cpu, void* ppu){};
    void (*naratte_clean_d)(void* cpu, void* ppu, void* dasm){};

    struct mem_change {
        struct mem_change* next;
        uint16_t addr;
        uint8_t data;
    };
    struct mem_change* (*naratte_get_mc)(void* cpu){};
    int8_t (*naratte_init_pseudo_mem)(void** mem, const char* boot, const char* game){};
    uint8_t (*mem_read)(void* mem, uint16_t addr){};
    void (*mem_write)(void* mem, uint16_t addr, uint8_t data){};

    std::vector<Instruction> _instructions;
    std::vector<MemWrites> _mem_writes;
};


#endif //NARATTEMIRU_NARATTE_H