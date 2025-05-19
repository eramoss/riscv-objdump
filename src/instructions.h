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


#define HAS_FIELD(inst, field) has_field_##field(&(inst))
static inline int has_field_rd(const instruction *inst) {
    return inst->type == REG_TYPE ||
           inst->type == IMM_TYPE ||
           inst->type == UPP_IMM_TYPE ||
           inst->type == JUMP_TYPE;
}

static inline int has_field_rs1(const instruction *inst) {
    return inst->type == REG_TYPE ||
           inst->type == IMM_TYPE ||
           inst->type == STORE_TYPE ||
           inst->type == BRANCH_TYPE;
}

static inline int has_field_rs2(const instruction *inst) {
    return inst->type == REG_TYPE ||
           inst->type == STORE_TYPE ||
           inst->type == BRANCH_TYPE;
}

static inline unsigned int btype_imm(instruction i){
	if (i.type == BRANCH_TYPE){
		unsigned int imm = 0;
		
		imm |= (i.b.imm_1_4 & 0xF) << 1;
		imm |= (i.b.imm_5_10 & 0x3F) << 5;
		imm |= (i.b.imm_11 & 0x1) << 11;
		imm |= (i.b.imm_12 & 0x1) << 12;

    if (imm & (1 << 12)) {
        imm |= 0xFFFFE000; 
    }
		return imm;
	}
	return 0;
}

static inline void update_btype_imm(instruction* i, unsigned int new_imm) {
	if (i->type != BRANCH_TYPE) return;

	unsigned int uimm = new_imm;

	i->b.imm_1_4 = (uimm >> 1) & 0xF;
	i->b.imm_5_10 = (uimm >> 5) & 0x3F;
	i->b.imm_11 = (uimm >> 11) & 0x1;
	i->b.imm_12 = (uimm >> 12) & 0x1;
}

static inline int is_dependent(instruction consumer, instruction producer){
	if (!HAS_FIELD(producer, rd) || producer.r.rd == 0)
		return 0; 

	if (!(HAS_FIELD(consumer, rs1) && consumer.r.rs1 == producer.r.rd)
		&& !(HAS_FIELD(consumer, rs2) && consumer.r.rs2 == producer.r.rd))
		return 0;

	return 1;
}
