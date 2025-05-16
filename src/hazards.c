#include "hazards.h"

void handle_hazards(rscv_asm_words asmw, int optimize_fowarding){
	int len = asmw.len;
	instruction* insts = decode_instructions(asmw);

	hazard_t* hazard_list = (hazard_t*)malloc(sizeof(hazard_t) * len);
	int num_hazards = identify_hazards(insts, len, hazard_list, optimize_fowarding);

	for (int i = 0; i < num_hazards; ++i) {
			printf("Hazard at instruction %d with %d: %s\n",
					hazard_list[i].index,
					hazard_list[i].hazard_with,
					hazard_list[i].type == DATA ? "data" :
					hazard_list[i].type == CTRL ? "Control" : "idk");
	}
}

int is_load_opcode(unsigned int opcode) {
  return opcode == 0x03;
}

int can_be_forwarded(const instruction producer, const instruction consumer) {
	if (!HAS_FIELD(producer, rd) || producer.r.rd == 0)
		return 0; 

	if (!(HAS_FIELD(consumer, rs1) && consumer.r.rs1 == producer.r.rd)
		&& !(HAS_FIELD(consumer, rs2) && consumer.r.rs2 == producer.r.rd))
		return 0;


	if (is_load_opcode(producer.r.opcode)) {
		return 0; 
	}

	return 1;
}

int identify_hazards(instruction *insts, int len, hazard_t *hazards, int optimize_forwarding) {
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
				if (optimize_forwarding) {
					instruction producer = insts[j];
					instruction consumer = insts[i];

					if (!can_be_forwarded(producer, consumer)) {
						hazards[count_hazards++] = (hazard_t){i, j, DATA};
					}
				} else {
					hazards[count_hazards++] = (hazard_t){i, j, DATA};
				}
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


void handle_nops(rscv_asm_words asmw, int optimize_forwarding){
	int len = asmw.len;
	instruction* insts = decode_instructions(asmw);

	hazard_t* hazard_list = (hazard_t*)malloc(sizeof(hazard_t) * len);
	int num_hazards = identify_hazards(insts, len, hazard_list,optimize_forwarding);
	for(int i = 0; i < len; i++){
		printf("0x%08x\n", asmw.words[i]);
		for(int j= num_hazards-1; j > -1; j--) {
			hazard_t h = hazard_list[j];

			if(h.hazard_with == i){
				if (h.type == DATA){
					int needs = 2;
					for(int num_nops = 0; num_nops < (needs - (h.index - (h.hazard_with + 1))); num_nops++)
						printf("0x00000013  ; nop\n");
					break;
				}
 				for(int num_nops = 0; num_nops < (h.index - h.hazard_with); num_nops++)
					printf("0x00000013  ; nop\n");
        break;
			}
		}
	}
}


int is_dependent(instruction consumer, instruction producer){
	if (!HAS_FIELD(producer, rd) || producer.r.rd == 0)
		return 0; 

	if (!(HAS_FIELD(consumer, rs1) && consumer.r.rs1 == producer.r.rd)
		&& !(HAS_FIELD(consumer, rs2) && consumer.r.rs2 == producer.r.rd))
		return 0;

	return 1;
}

int32_t _btype_imm(instruction i){
	if (i.type == BRANCH_TYPE){
		int32_t imm = 0;
		
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

void update_btype_imm(instruction* i, int32_t new_imm) {
	if (i->type != BRANCH_TYPE) return;

	int32_t uimm = new_imm;

	i->b.imm_1_4 = (uimm >> 1) & 0xF;
	i->b.imm_5_10 = (uimm >> 5) & 0x3F;
	i->b.imm_11 = (uimm >> 11) & 0x1;
	i->b.imm_12 = (uimm >> 12) & 0x1;
}

rscv_asm_words reorder(rscv_asm_words asmw, int optimize_fowarding){
  rscv_asm_words result;
	result.len = asmw.len;
	memcpy(result.words, asmw.words, result.len* sizeof(word_t));

	int len = result.len;
	instruction* insts = decode_instructions(result);

	hazard_t* hazard_list = (hazard_t*)malloc(sizeof(hazard_t) * len);
	int num_hazards = identify_hazards(insts, len, hazard_list, optimize_fowarding);

	int window = 0;
	int window_start = 0;
	for (int i = 0; i < len; i++){
		instruction current_inst = insts[i];
		if (current_inst.type == BRANCH_TYPE){
			window = _btype_imm(current_inst) /4;
			window_start = i;
			continue;
		}
			
		int stop_at = window ? window + window_start: len;

		if (stop_at == i) window = 0;

		for(int j= num_hazards-1; j > -1; j--) {
			hazard_t h = hazard_list[j];
			if(h.index == i && h.type == DATA){
				int temp_i = i;
				int count = 0;
				for(int k = i + 1; k<stop_at; k++){
					instruction candidate = insts[k];
					if (is_dependent(candidate, current_inst)) break;
					if (candidate.type == BRANCH_TYPE)break;
					if (count == 2) break;
					
					word_t temp = result.words[temp_i];
					result.words[temp_i] = result.words[k];
					result.words[k] = temp;
					temp_i++;
					count++;
				}
			}
		}
	}

	return result;
}
