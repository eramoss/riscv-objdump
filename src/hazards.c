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
					hazard_list[i].type == RAW ? "RAW" :
					hazard_list[i].type == WAW ? "WAW" : "WAR");
	}
}

int identify_hazards(instruction *insts, int len, hazard_t *hazards) {
    int count_hazards = 0;

    for (int i = 0; i < len; i++) {
        bitfield src1 = 0, src2 = 0, dest = 0;

        switch (insts[i].type) {
            case REG_TYPE:
                src1 = insts[i].r.rs1;
                src2 = insts[i].r.rs2;
                dest = insts[i].r.rd;
                break;
            case IMM_TYPE:
                src1 = insts[i].i.rs1;
                dest = insts[i].i.rd;
                break;
            case STORE_TYPE:
                src1 = insts[i].s.rs1;
                src2 = insts[i].s.rs2;
                break;
            case BRANCH_TYPE:
                src1 = insts[i].b.rs1;
                src2 = insts[i].b.rs2;
                break;
            case UPP_IMM_TYPE:
                dest = insts[i].u.rd;
                break;
            case JUMP_TYPE:
                dest = insts[i].j.rd;
                break;
            default:
                break;
        }

        for (int j = i - 1; j >= i - 4 && j >= 0; j--) {
            bitfield prev_src1 = 0, prev_src2 = 0, prev_dest = 0;

            switch (insts[j].type) {
                case REG_TYPE:
                    prev_src1 = insts[j].r.rs1;
                    prev_src2 = insts[j].r.rs2;
                    prev_dest = insts[j].r.rd;
                    break;
                case IMM_TYPE:
                    prev_src1 = insts[j].i.rs1;
                    prev_dest = insts[j].i.rd;
                    break;
                case STORE_TYPE:
                    prev_src1 = insts[j].s.rs1;
                    prev_src2 = insts[j].s.rs2;
                    break;
                case BRANCH_TYPE:
                    prev_src1 = insts[j].b.rs1;
                    prev_src2 = insts[j].b.rs2;
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
                hazards[count_hazards++] = (hazard_t){i, j, RAW};
            }

            if (dest != 0 && dest == prev_dest) {
                hazards[count_hazards++] = (hazard_t){i, j, WAW};
            }

            if (dest != 0 && (dest == prev_src1 || dest == prev_src2)) {
                hazards[count_hazards++] = (hazard_t){i, j, WAR};
            }
        }
    }

    return count_hazards;
}
