#pragma once
#include "instructions.h"
#include "words.h"
#include "opcode.h"

typedef enum {
	RAW,
	WAW,
	WAR
} hazard_type;

typedef struct {
	int index;
	int hazard_with;
	hazard_type type;
} hazard_t;


void handle_hazards(rscv_asm_words asmw);
int identify_hazards(instruction * insts, int len, hazard_t* hazards);
