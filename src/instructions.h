#pragma once
#include <string.h>
#include "typedefs.h"

typedef unsigned int bitfield;

typedef enum  {
	unknown = -1,
	BRANCH_TYPE,
	REG_TYPE,
	STORE_TYPE,
	JUMP_TYPE,
	UPP_IMM_TYPE,
	IMM_TYPE
} INSTYPES;

typedef struct {
    bitfield opcode : 7;
    bitfield rd     : 5;
    bitfield funct3 : 3;
    bitfield rs1    : 5;
    bitfield rs2    : 5;
    bitfield funct7 : 7;
} RType;

typedef struct {
    bitfield opcode : 7;
    bitfield rd     : 5;
    bitfield funct3 : 3;
    bitfield rs1    : 5;
    bitfield imm    : 12;
} IType;

typedef struct {
    bitfield opcode   : 7;
    bitfield imm_0_4  : 5;
    bitfield funct3   : 3;
    bitfield rs1      : 5;
    bitfield rs2      : 5;
    bitfield imm_5_11 : 7;
} SType;

typedef struct {
    bitfield opcode   : 7;
    bitfield imm_11   : 1;
    bitfield imm_1_4  : 4;
    bitfield funct3   : 3;
    bitfield rs1      : 5;
    bitfield rs2      : 5;
    bitfield imm_5_10 : 6;
    bitfield imm_12   : 1;
} BType;

typedef struct {
    bitfield opcode : 7;
    bitfield rd     : 5;
    bitfield imm    : 20;
} UType;

typedef struct {
    bitfield opcode    : 7;
    bitfield rd        : 5;
    bitfield imm_12_19 : 8;
    bitfield imm_11    : 1;
    bitfield imm_1_10  : 10;
    bitfield imm_20    : 1;
} JType;

typedef struct {
    INSTYPES type;
    union {
        RType r;
        IType i;
        SType s;
        BType b;
        UType u;
        JType j;
        word_t raw; 
    };
} instruction;


#define HAS_FIELD(inst, field) _has_field(&(inst), #field)

static inline int _has_field(const instruction *inst, const char *field) {
    switch (inst->type) {
        case REG_TYPE:
            return strcmp(field, "opcode") == 0 ||
                   strcmp(field, "rd") == 0 ||
                   strcmp(field, "funct3") == 0 ||
                   strcmp(field, "rs1") == 0 ||
                   strcmp(field, "rs2") == 0 ||
                   strcmp(field, "funct7") == 0;

        case IMM_TYPE:
            return strcmp(field, "opcode") == 0 ||
                   strcmp(field, "rd") == 0 ||
                   strcmp(field, "funct3") == 0 ||
                   strcmp(field, "rs1") == 0 ||
                   strcmp(field, "imm") == 0;

        case STORE_TYPE:
            return strcmp(field, "opcode") == 0 ||
                   strcmp(field, "imm_0_4") == 0 ||
                   strcmp(field, "funct3") == 0 ||
                   strcmp(field, "rs1") == 0 ||
                   strcmp(field, "rs2") == 0 ||
                   strcmp(field, "imm_5_11") == 0;

        case BRANCH_TYPE:
            return strcmp(field, "opcode") == 0 ||
                   strcmp(field, "imm_11") == 0 ||
                   strcmp(field, "imm_1_4") == 0 ||
                   strcmp(field, "funct3") == 0 ||
                   strcmp(field, "rs1") == 0 ||
                   strcmp(field, "rs2") == 0 ||
                   strcmp(field, "imm_5_10") == 0 ||
                   strcmp(field, "imm_12") == 0;

        case UPP_IMM_TYPE:
            return strcmp(field, "opcode") == 0 ||
                   strcmp(field, "rd") == 0 ||
                   strcmp(field, "imm") == 0;

        case JUMP_TYPE:
            return strcmp(field, "opcode") == 0 ||
                   strcmp(field, "rd") == 0 ||
                   strcmp(field, "imm_12_19") == 0 ||
                   strcmp(field, "imm_11") == 0 ||
                   strcmp(field, "imm_1_10") == 0 ||
                   strcmp(field, "imm_20") == 0;

        default:
            return 0;
    }
}
