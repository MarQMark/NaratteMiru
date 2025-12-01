#include "disassembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REG_B 0b000
#define REG_C 0b001
#define REG_D 0b010
#define REG_E 0b011
#define REG_H 0b100
#define REG_L 0b101
#define REG_M 0b110
#define REG_A 0b111

#define REG_BC 0b00
#define REG_DE 0b01
#define REG_HL 0b10
#define REG_AF 0b11
#define REG_SP 0b11

#define MASK_012 0b00000111
#define MASK_345 0b00111000
#define MASK_45  0b00110000
#define MASK_34  0b00011000

char dasm_get_reg8(const uint8_t id) {
    switch (id) {
        case REG_B:
            return 'B';
        case REG_C:
            return 'C';
        case REG_D:
            return 'D';
        case REG_E:
            return 'E';
        case REG_H:
            return 'H';
        case REG_L:
            return 'L';
        case REG_M:
            return 'M';
        case REG_A:
            return 'A';
        default:
            printf("Unknown register %u", id);
            return 'X';
    }
}

const char* dasm_get_reg16_AF(const uint8_t id) {
    switch (id) {
        case REG_BC:
            return "BC";
        case REG_DE:
            return "DE";
        case REG_HL:
            return "HL";
        case REG_AF:
            return "AF";
        default:
            printf("Unknown register %u", id);
            return "XX";
    }
}

const char* dasm_get_reg16_SP(const uint8_t id) {
    switch (id) {
        case REG_BC:
            return "BC";
        case REG_DE:
            return "DE";
        case REG_HL:
            return "HL";
        case REG_AF:
            return "SP";
        default:
            printf("Unknown register %u", id);
            return "XX";
    }
}

void dasm_LD_r_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD %c, %c",
        dasm_get_reg8((disasm->ic & MASK_345) >> 3),
        dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_LD_r_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;

    snprintf(disasm->buf, sizeof(disasm->buf), "LD %c, 0x%X",
        dasm_get_reg8((disasm->ic & MASK_345) >> 3),
        z);
}

void dasm_LD_r_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD %c, [HL]",
        dasm_get_reg8((disasm->ic & MASK_345) >> 3));
}

void dasm_LD_HL_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD [HL], %c",
            dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_LD_HL_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "LD [HL], 0x%X",z);
}

void dasm_LD_A_BC(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD A, [BC]");
}

void dasm_LD_A_DE(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD A, [DE]");
}

void dasm_LD_BC_A(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD [BC], A");
}

void dasm_LD_DE_A(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD [DE], A");
}

void dasm_LD_A_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "LD A, 0x%X",z);
}

void dasm_LD_A_nn(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    const uint8_t w = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "LD A, [0x%X]", ((uint16_t)w) << 8 | z);
}

void dasm_LD_nn_A(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    const uint8_t w = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "LD [0x%X], A", ((uint16_t)w) << 8 | z);
}

void dasm_LDH_A_C(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LDH A, [0xFF + C]");
}

void dasm_LDH_C_A(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LDH [0xFF + C], A");
}

void dasm_LDH_A_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "LDH A, [0xFF%02X]", z);
}

void dasm_LDH_n_A(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "LDH [0xFF%02X], A", z);
}

void dasm_LD_A_HL_dec(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD A, [HL--]");
}

void dasm_LD_HL_A_dec(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD [HL--], A");
}

void dasm_LD_A_HL_inc(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD A, [HL++]");
}

void dasm_LD_HL_A_inc(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD [HL++], A");
}

void dasm_LD_rr_nn(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    const uint8_t w = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "LD %s, 0x%X",
        dasm_get_reg16_SP((disasm->ic & MASK_45) >> 4),
        (uint16_t)w << 8 | z);
}

void dasm_LD_nn_SP(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    const uint8_t w = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "LD [0x%X], SP", (uint16_t)w << 8 | z);
}

void dasm_LD_SP_nn(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    const uint8_t w = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "LD SP, 0x%X", (uint16_t)w << 8 | z);
}

void dasm_LD_SP_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "LD SP, HL");
}

void dasm_PUSH_rr(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "PUSH [SP-2], %s",
        dasm_get_reg16_AF((disasm->ic & MASK_45) >> 4));
}

void dasm_POP_rr(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "PUSH %s, [SP+2]",
       dasm_get_reg16_AF((disasm->ic & MASK_45) >> 4));
}

void dasm_LD_HL_SP_e(struct dasm* disasm) {
    const int8_t z = (int8_t)disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "LD HL SP+0x%X", z);
}

void dasm_ADD_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "ADD A, %c",
        dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_ADD_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "ADD A, [HL]");
}

void dasm_ADD_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "ADD A, 0x%X", z);
}

void dasm_ADC_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "ADC A, %c",
        dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_ADC_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "ADC A, [HL]");
}

void dasm_ADC_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "ADC A, 0x%X", z);
}

void dasm_SUB_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SUB A, %c",
        dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_SUB_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SUB A, [HL]");
}

void dasm_SUB_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "SUB A, 0x%X", z);
}

void dasm_SBC_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SBC A, %c",
        dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_SBC_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SBC A, [HL]");
}

void dasm_SBC_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "SBC A, 0x%X", z);
}

void dasm_CP_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "CP A, %c",
    dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_CP_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "CP A, [HL]");
}

void dasm_CP_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "CP A, 0x%X", z);
}

void dasm_INC_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "INC %c",
        dasm_get_reg8((disasm->ic & MASK_345) >> 3));
}

void dasm_INC_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "INC [HL]");
}

void dasm_DEC_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "DEC %c",
        dasm_get_reg8((disasm->ic & MASK_345) >> 3));
}

void dasm_DEC_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "DEC [HL]");
}

void dasm_AND_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "AND A, %c",
    dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_AND_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "AND A, [HL]");
}

void dasm_AND_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "AND A, 0x%X", z);
}

void dasm_OR_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "OR A, %c",
    dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_OR_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "OR A, [HL]");
}

void dasm_OR_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "OR A, 0x%X", z);
}

void dasm_XOR_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "XOR A, %c",
        dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_XOR_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "XOR A, [HL]");
}

void dasm_XOR_n(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "XOR A, 0x%X", z);
}

void dasm_CCF(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "CCF");
}

void dasm_SCF(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SCF");
}

void dasm_DAA(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "DAA");
}

void dasm_CPL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "CPL");
}

void dasm_INC_rr(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "INC %s",
        dasm_get_reg16_SP((disasm->ic & MASK_45) >> 4));
}

void dasm_DEC_rr(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "DEC %s",
            dasm_get_reg16_SP((disasm->ic & MASK_45) >> 4));
}

void dasm_ADD_HL_rr(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "ADD HL, %s",
        dasm_get_reg16_SP((disasm->ic & MASK_45) >> 4));
}

void dasm_ADD_SP_e(struct dasm* disasm) {
    const int8_t z = (int8_t)disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "ADD SP+0x%X", z);
}

void dasm_RLCA(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RLCA");
}

void dasm_RRCA(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RRCA");
}

void dasm_RLA(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RLA");
}

void dasm_RRA(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RRA");
}

void dasm_RLC_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RLC %c",
        dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_RLC_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RLC [HL]");
}

void dasm_RRC_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RRC %c",
            dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_RRC_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RRC [HL]");
}

void dasm_RL_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RL %c",
            dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_RL_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RL [HL]");
}

void dasm_RR_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RR %c",
            dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_RR_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RR [HL]");
}

void dasm_SLA_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SLA %c",
            dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_SLA_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SLA [HL]");
}

void dasm_SRA_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SRA %c",
            dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_SRA_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SRA [HL]");
}

void dasm_SWAP_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SWAP %c",
            dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_SWAP_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SWAP [HL]");
}

void dasm_SRL_r(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SRL %c",
            dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_SRL_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "SRL [HL]");
}

void dasm_BIT_b_r(struct dasm* disasm) {
    const uint8_t b = (disasm->ic & MASK_345) >> 3;
    snprintf(disasm->buf, sizeof(disasm->buf), "BIT 0x%X %c", b,
            dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_BIT_b_HL(struct dasm* disasm) {
    const uint8_t b = (disasm->ic & MASK_345) >> 3;
    snprintf(disasm->buf, sizeof(disasm->buf), "BIT 0x%X [HL]", b);
}

void dasm_RES_b_r(struct dasm* disasm) {
    const uint8_t b = (disasm->ic & MASK_345) >> 3;
    snprintf(disasm->buf, sizeof(disasm->buf), "RES 0x%X %c", b,
            dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_RES_b_HL(struct dasm* disasm) {
    const uint8_t b = (disasm->ic & MASK_345) >> 3;
    snprintf(disasm->buf, sizeof(disasm->buf), "RES 0x%X [HL]", b);
}

void dasm_SET_BIT_r(struct dasm* disasm) {
    const uint8_t b = (disasm->ic & MASK_345) >> 3;
    snprintf(disasm->buf, sizeof(disasm->buf), "SET 0x%X %c", b,
            dasm_get_reg8(disasm->ic & MASK_012));
}

void dasm_set_BIT_HL(struct dasm* disasm) {
    const uint8_t b = (disasm->ic & MASK_345) >> 3;
    snprintf(disasm->buf, sizeof(disasm->buf), "SET 0x%X [HL]", b);
}

void dasm_JP_nn(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    const uint8_t w = disasm->instructions[disasm->PC];
    disasm->PC++;
    //disasm->PC = ((uint16_t)w << 8) | z;
    snprintf(disasm->buf, sizeof(disasm->buf), "JP 0x%X", ((uint16_t)w << 8) | z);
}

void dasm_JP_HL(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "JP [HL]");
}

void dasm_JP_cc_nn(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    const uint8_t w = disasm->instructions[disasm->PC];
    disasm->PC++;

    const uint8_t b = (disasm->ic & MASK_34) >> 3;
    switch (b) {
        case 0b00: // NZ
            snprintf(disasm->buf, sizeof(disasm->buf), "JP NZ 0x%X", ((uint16_t)w << 8) | z);
            break;
        case 0b01: // Z
            snprintf(disasm->buf, sizeof(disasm->buf), "JP Z 0x%X", ((uint16_t)w << 8) | z);
            break;
        case 0b10: // NC
            snprintf(disasm->buf, sizeof(disasm->buf), "JP NC 0x%X", ((uint16_t)w << 8) | z);
            break;
        case 0b11: // C
            snprintf(disasm->buf, sizeof(disasm->buf), "JP C 0x%X", ((uint16_t)w << 8) | z);
            break;
        default:
            snprintf(disasm->buf, sizeof(disasm->buf), "[ERROR] JP cc nn, invalid condition");
    }
}

void dasm_JR_e(struct dasm* disasm) {
    const int8_t z = (int8_t)disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "JR 0x%X (%d)", (uint8_t)z, z);
}

void dasm_JR_cc_e(struct dasm* disasm) {
    const int8_t z = (int8_t)disasm->instructions[disasm->PC];
    disasm->PC++;

    const uint8_t b = (disasm->ic & MASK_34) >> 3;
    switch (b) {
        case 0b00: // NZ
            snprintf(disasm->buf, sizeof(disasm->buf), "JR NZ 0x%X (%d)", (uint8_t)z, z);
            break;
        case 0b01: // Z
            snprintf(disasm->buf, sizeof(disasm->buf), "JR Z 0x%X (%d)", (uint8_t)z, z);
            break;
        case 0b10: // NC
            snprintf(disasm->buf, sizeof(disasm->buf), "JR NC 0x%X (%d)", (uint8_t)z, z);
            break;
        case 0b11: // C
            snprintf(disasm->buf, sizeof(disasm->buf), "JR C 0x%X (%d)", (uint8_t)z, z);
            break;
        default:
            snprintf(disasm->buf, sizeof(disasm->buf), "[ERROR] JR cc e, invalid condition");
    }
}

void dasm_CALL_nn(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    const uint8_t w = disasm->instructions[disasm->PC];
    disasm->PC++;
    snprintf(disasm->buf, sizeof(disasm->buf), "CALL 0x%X", ((uint16_t)w << 8) | z);
}

void dasm_CALL_cc_nn(struct dasm* disasm) {
    const uint8_t z = disasm->instructions[disasm->PC];
    disasm->PC++;
    const uint8_t w = disasm->instructions[disasm->PC];
    disasm->PC++;

    const uint8_t b = (disasm->ic & MASK_34) >> 3;
    switch (b) {
        case 0b00: // NZ
            snprintf(disasm->buf, sizeof(disasm->buf), "CALL NZ 0x%X", ((uint16_t)w << 8) | z);
            break;
        case 0b01: // Z
            snprintf(disasm->buf, sizeof(disasm->buf), "CALL Z 0x%X", ((uint16_t)w << 8) | z);
            break;
        case 0b10: // NC
            snprintf(disasm->buf, sizeof(disasm->buf), "CALL NC 0x%X", ((uint16_t)w << 8) | z);
            break;
        case 0b11: // C
            snprintf(disasm->buf, sizeof(disasm->buf), "CALL C 0x%X", ((uint16_t)w << 8) | z);
            break;
        default:
            snprintf(disasm->buf, sizeof(disasm->buf), "[ERROR] CALL cc nn, invalid condition");
    }
}

void dasm_RET(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RET");
}

void dasm_RET_cc(struct dasm* disasm) {
    const uint8_t b = (disasm->ic & MASK_34) >> 3;
    switch (b) {
        case 0b00: // NZ
            snprintf(disasm->buf, sizeof(disasm->buf), "RET NZ");
            break;
        case 0b01: // Z
            snprintf(disasm->buf, sizeof(disasm->buf), "RET Z");
            break;
        case 0b10: // NC
            snprintf(disasm->buf, sizeof(disasm->buf), "RET NC");
            break;
        case 0b11: // C
            snprintf(disasm->buf, sizeof(disasm->buf), "RET C");
            break;
        default:
            snprintf(disasm->buf, sizeof(disasm->buf), "[ERROR] RET cc, invalid condition");
    }
}

void dasm_RETI(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RETI");
}

void dasm_RST_n(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "RST 0x%X", disasm->ic & 0b00111000);
}

void dasm_DI(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "DI");
}

void dasm_EI(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "EI");
}

void dasm_HALT(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "HALT");
}

void dasm_STOP(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "STOP");
}

void dasm_NOP(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "NOP");
}

void dsam_no_ist(struct dasm* disasm) {
    snprintf(disasm->buf, sizeof(disasm->buf), "No valid instruction\n\t0x%X \n", disasm->ic);
}

void dasm_exec_cb(struct dasm* disasm) {
    disasm->ic = disasm->instructions[disasm->PC];
    disasm->PC++;
    disasm->decode_cb[disasm->ic](disasm);
}

int8_t dasm_init(struct dasm** disasm) {
    *disasm = malloc(sizeof(struct dasm));
    if (*disasm == NULL)
        return -1;

    memset(*disasm, 0, sizeof(struct dasm));

    for (uint16_t i = 0; i < 256; i++)
        (*disasm)->decode[i] = dsam_no_ist;

    for (uint8_t ins = 0b01000000; ins <= 0b01111111; ins++)
        (*disasm)->decode[ins] = dasm_LD_r_r;

    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[(reg << 3) | 0b00000110] = dasm_LD_r_n;

    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[(reg << 3) | 0b01000110] = dasm_LD_r_HL;

    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[reg | 0b01110000] = dasm_LD_HL_r;

    (*disasm)->decode[0b00110110] = dasm_LD_HL_n;

    (*disasm)->decode[0b00001010] = dasm_LD_A_BC;
    (*disasm)->decode[0b00011010] = dasm_LD_A_DE;

    (*disasm)->decode[0b00000010] = dasm_LD_BC_A;
    (*disasm)->decode[0b00010010] = dasm_LD_DE_A;

    (*disasm)->decode[0b00111110] = dasm_LD_A_n;

    (*disasm)->decode[0b11111010] = dasm_LD_A_nn;
    (*disasm)->decode[0b11101010] = dasm_LD_nn_A;

    (*disasm)->decode[0b11110010] = dasm_LDH_A_C;
    (*disasm)->decode[0b11100010] = dasm_LDH_C_A;

    (*disasm)->decode[0b11110000] = dasm_LDH_A_n;
    (*disasm)->decode[0b11100000] = dasm_LDH_n_A;

    (*disasm)->decode[0b00111010] = dasm_LD_A_HL_dec;
    (*disasm)->decode[0b00110010] = dasm_LD_HL_A_dec;

    (*disasm)->decode[0b00101010] = dasm_LD_A_HL_inc;
    (*disasm)->decode[0b00100010] = dasm_LD_HL_A_inc;

    for (uint8_t reg = 0; reg <=0b11; reg++)
        (*disasm)->decode[(reg << 4) | 0b00000001] = dasm_LD_rr_nn;

    (*disasm)->decode[0b00001000] = dasm_LD_nn_SP;
    (*disasm)->decode[0b00110001] = dasm_LD_SP_nn;
    (*disasm)->decode[0b11111001] = dasm_LD_SP_HL;

    for (uint8_t reg = 0; reg <= 0b11; reg++)
        (*disasm)->decode[(reg << 4) | 0b11000101] = dasm_PUSH_rr;
    for (uint8_t reg = 0; reg <= 0b11; reg++)
        (*disasm)->decode[(reg << 4) | 0b11000001] = dasm_POP_rr;

    (*disasm)->decode[0b11111000] = dasm_LD_HL_SP_e;

    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[reg | 0b10000000] = dasm_ADD_r;
    (*disasm)->decode[0b10000110] = dasm_ADD_HL;
    (*disasm)->decode[0b11000110] = dasm_ADD_n;
    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[reg | 0b10001000] = dasm_ADC_r;
    (*disasm)->decode[0b10001110] = dasm_ADC_HL;
    (*disasm)->decode[0b11001110] = dasm_ADC_n;

    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[reg | 0b10010000] = dasm_SUB_r;
    (*disasm)->decode[0b10010110] = dasm_SUB_HL;
    (*disasm)->decode[0b11010110] = dasm_SUB_n;
    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[reg | 0b10011000] = dasm_SBC_r;
    (*disasm)->decode[0b10011110] = dasm_SBC_HL;
    (*disasm)->decode[0b11011110] = dasm_SBC_n;

    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[reg | 0b10111000] = dasm_CP_r;
    (*disasm)->decode[0b10111110] = dasm_CP_HL;
    (*disasm)->decode[0b11111110] = dasm_CP_n;

    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[(reg << 3) | 0b00000100] = dasm_INC_r;
    (*disasm)->decode[0b00110100] = dasm_INC_HL;
    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[(reg << 3) | 0b00000101] = dasm_DEC_r;
    (*disasm)->decode[0b00110101] = dasm_DEC_HL;

    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode[reg | 0b10100000] = dasm_AND_r;
    (*disasm)->decode[0b10100110] = dasm_AND_HL;
    (*disasm)->decode[0b11100110] = dasm_AND_n;
    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[reg | 0b10110000] = dasm_OR_r;
    (*disasm)->decode[0b10110110] = dasm_OR_HL;
    (*disasm)->decode[0b11110110] = dasm_OR_n;
    for (uint8_t reg = 0; reg <= 0b111; reg++)
        (*disasm)->decode[reg | 0b10101000] = dasm_XOR_r;
    (*disasm)->decode[0b10101110] = dasm_XOR_HL;
    (*disasm)->decode[0b11101110] = dasm_XOR_n;
    (*disasm)->decode[0b00111111] = dasm_CCF;
    (*disasm)->decode[0b00110111] = dasm_SCF;
    (*disasm)->decode[0b00100111] = dasm_DAA;
    (*disasm)->decode[0b00101111] = dasm_CPL;

    for (uint8_t reg = 0; reg <=0b11; reg++)
        (*disasm)->decode[(reg << 4) | 0b00000011] = dasm_INC_rr;
    for (uint8_t reg = 0; reg <=0b11; reg++)
        (*disasm)->decode[(reg << 4) | 0b00001011] = dasm_DEC_rr;
    for (uint8_t reg = 0; reg <=0b11; reg++)
        (*disasm)->decode[(reg << 4) | 0b00001001] = dasm_ADD_HL_rr;
    (*disasm)->decode[0b11101000] = dasm_ADD_SP_e;

    (*disasm)->decode[0b00000111] = dasm_RLCA;
    (*disasm)->decode[0b00001111] = dasm_RRCA;
    (*disasm)->decode[0b00010111] = dasm_RLA;
    (*disasm)->decode[0b00011111] = dasm_RRA;

    (*disasm)->decode[0xCB] = dasm_exec_cb;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode_cb[reg | 0b00000000] = dasm_RLC_r;
    (*disasm)->decode_cb[0b00000110] = dasm_RLC_HL;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode_cb[reg | 0b00001000] = dasm_RRC_r;
    (*disasm)->decode_cb[0b00001110] = dasm_RRC_HL;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode_cb[reg | 0b00010000] = dasm_RL_r;
    (*disasm)->decode_cb[0b00010110] = dasm_RL_HL;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode_cb[reg | 0b00011000] = dasm_RR_r;
    (*disasm)->decode_cb[0b00011110] = dasm_RR_HL;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode_cb[reg | 0b00100000] = dasm_SLA_r;
    (*disasm)->decode_cb[0b00100110] = dasm_SLA_HL;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode_cb[reg | 0b00101000] = dasm_SRA_r;
    (*disasm)->decode_cb[0b00101110] = dasm_SRA_HL;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode_cb[reg | 0b00110000] = dasm_SWAP_r;
    (*disasm)->decode_cb[0b00110110] = dasm_SWAP_HL;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode_cb[reg | 0b00111000] = dasm_SRL_r;
    (*disasm)->decode_cb[0b00111110] = dasm_SRL_HL;
    for (uint8_t reg = 0; reg <=0b111111; reg++)
        (*disasm)->decode_cb[reg | 0b01000000] = dasm_BIT_b_r;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode_cb[(reg << 3) | 0b01000110] = dasm_BIT_b_HL;
    for (uint8_t reg = 0; reg <=0b111111; reg++)
        (*disasm)->decode_cb[reg | 0b10000000] = dasm_RES_b_r;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode_cb[(reg << 3) | 0b10000110] = dasm_RES_b_HL;
    for (uint8_t reg = 0; reg <=0b111111; reg++)
        (*disasm)->decode_cb[reg | 0b11000000] = dasm_SET_BIT_r;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode_cb[(reg << 3) | 0b11000110] = dasm_set_BIT_HL;

    (*disasm)->decode[0b11000011] = dasm_JP_nn;
    (*disasm)->decode[0b11101001] = dasm_JP_HL;
    for (uint8_t reg = 0; reg <=0b11; reg++)
        (*disasm)->decode[(reg << 3) | 0b11000010] = dasm_JP_cc_nn;
    (*disasm)->decode[0b00011000] = dasm_JR_e;
    for (uint8_t reg = 0; reg <=0b11; reg++)
        (*disasm)->decode[(reg << 3) | 0b00100000] = dasm_JR_cc_e;
    (*disasm)->decode[0b11001101] = dasm_CALL_nn;
    for (uint8_t reg = 0; reg <=0b11; reg++)
        (*disasm)->decode[(reg << 3) | 0b11000100] = dasm_CALL_cc_nn;
    (*disasm)->decode[0b11001001] = dasm_RET;
    for (uint8_t reg = 0; reg <=0b11; reg++)
        (*disasm)->decode[(reg << 3) | 0b11000000] = dasm_RET_cc;
    (*disasm)->decode[0b11011001] = dasm_RETI;
    for (uint8_t reg = 0; reg <=0b111; reg++)
        (*disasm)->decode[(reg << 3) | 0b11000111] = dasm_RST_n;

    (*disasm)->decode[0b11110011] = dasm_DI;
    (*disasm)->decode[0b11111011] = dasm_EI;
    (*disasm)->decode[0b01110110] = dasm_HALT;
    (*disasm)->decode[0b00010000] = dasm_STOP;
    (*disasm)->decode[0b00000000] = dasm_NOP;

    return 1;
}

void dasm_free(struct dasm **disasm) {
    if (!(*disasm))
        return;

    free(*disasm);
    (*disasm) = NULL;
}

void dasm_disassemble(struct dasm *disasm, uint8_t *ins) {
    disasm->instructions = malloc(3);
    memcpy(disasm->instructions, ins, 3);
    disasm->PC = 0;

    disasm->ic = disasm->instructions[disasm->PC];
    disasm->PC++;

    memset(disasm->buf, 0, sizeof(disasm->buf));
    disasm->decode[disasm->ic](disasm);

    free(disasm->instructions);
}
