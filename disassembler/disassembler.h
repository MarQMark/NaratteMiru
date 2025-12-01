#ifndef NARATTE_DISASSAMBLER_H
#define NARATTE_DISASSAMBLER_H


#include <stdint.h>

struct dasm {
    uint8_t* instructions;
    uint8_t ic;
    uint16_t PC;

    char buf[64];

    void (*decode[256])(struct dasm*);
    void (*decode_cb[256])(struct dasm*);
};

int8_t dasm_init(struct dasm** disasm);
void dasm_free(struct dasm** disasm);
void dasm_disassemble(struct dasm* disasm, uint8_t* ins);

#endif //NARATTE_DISASSAMBLER_H