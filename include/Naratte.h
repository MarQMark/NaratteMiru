#ifndef NARATTEMIRU_NARATTE_H
#define NARATTEMIRU_NARATTE_H

#include <cstdint>
#include <vector>

struct Instruction {
    uint8_t op[3] = {0xFD, 0xFD, 0xFD}; // Invalid Opcodes
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

private:
    void* _lib_handle{};

    void* _cpu{};
    void* _ppu{};
    void* _dasm{};

    //int8_t (*naratte_init)(void** cpu, void** ppu){};
    int8_t (*naratte_init_d)(void** cpu, void** ppu, void** dasm){};
    int8_t (*naratte_load_rom)(void* cpu, const char* boot, const char* game){};

    void (*naratte_tick)(void* cpu, void* ppu){};

    void (*naratte_get_ic)(void *cpu, uint8_t* ic);
    char* (*naratte_disassemble)(void* dasm, uint8_t *ins);
    char* (*naratte_disassemble_cpu)(void* cpu, void* dasm){};

    //void (*naratte_clean)(void* cpu, void* ppu){};
    void (*naratte_clean_d)(void* cpu, void* ppu, void* dasm){};


    std::vector<Instruction> _instructions;
};


#endif //NARATTEMIRU_NARATTE_H