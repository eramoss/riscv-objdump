#include "hazards.h"

void handle_hazards(rscv_asm_words asmw){
	int len = asmw.len;
	instruction* insts = decode_instructions(asmw);

	hazard_t* hazard_list = (hazard_t*)malloc(sizeof(hazard_t) * len);
	int num_hazards = identify_hazards(insts, len, hazard_list);

	for (int i = 0; i < num_hazards; ++i) {
			printf("Hazard at instruction %d with %d: %s\n",
					hazard_list[i].index,
					hazard_list[i].hazard_with,
					hazard_list[i].type == DATA ? "data" :
					hazard_list[i].type == CTRL ? "Control" : "idk");
	}
}

int identify_hazards(instruction *insts, int len, hazard_t *hazards) {
	int count_hazards = 0;

	for (int i = 0; i < len; i++) {
		bitfield src1 = 0, src2 = 0;

		switch (insts[i].type) {
			case REG_TYPE:
				src1 = insts[i].r.rs1;
				src2 = insts[i].r.rs2;
				break;
			case IMM_TYPE:
				src1 = insts[i].i.rs1;
				break;
			case STORE_TYPE:
				src1 = insts[i].s.rs1;
				src2 = insts[i].s.rs2;
				break;
			case BRANCH_TYPE:
				src1 = insts[i].b.rs1;
				src2 = insts[i].b.rs2;
				break;
			default:
				break;
		}
		// DATA HAZARD
		for (int j = i - 1; j >= i - 2 && j >= 0; j--) {
			bitfield prev_dest = 0;

			switch (insts[j].type) {
				case REG_TYPE:
					prev_dest = insts[j].r.rd;
					break;
				case IMM_TYPE:
					prev_dest = insts[j].i.rd;
					break;
				case UPP_IMM_TYPE:
					prev_dest = insts[j].u.rd;
					break;
				case JUMP_TYPE:
					prev_dest = insts[j].j.rd;
					break;
				default:
					break;
			}

			if ((src1 == prev_dest && src1 != 0) || (src2 == prev_dest && src2 != 0)) {
				hazards[count_hazards++] = (hazard_t){i, j, DATA};
			}
		}
		// CTRL HAZARD
		if (insts[i].type == BRANCH_TYPE) {
			for (int j = i + 1; j < i + 4 && j < len; j++){
				hazards[count_hazards++] = (hazard_t){j,i, CTRL};
			}
		}
	}

	return count_hazards;
}
