#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instructions.h"
#include "typedefs.h"
#include "words.h"

#define B_TYPE 0b1100011
#define R_TYPE 0b0110011
#define S_TYPE 0b0100011
#define J_TYPE 0b1101111

// itypes
#define FEN_I_TYPE 0b0001111
#define OP_I_TYPE 0b1110011
#define LI_TYPE 0b0000011
#define I_TYPE 0b0010011

// utypes
#define LUI 0b0110111
#define AUIPC 0b0010111

INSTYPES indentify_op_type(u32bit_t b);
void handle_types(rscv_asm_words asmw);
const char* riscv_instype_str(INSTYPES type);
