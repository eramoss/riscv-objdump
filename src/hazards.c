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
				case STORE_TYPE:
					prev_dest = insts[j].s.rs1;
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
void nops_on_ctrls(rscv_asm_words *asmw){
	int len = asmw->len;
	instruction* insts = decode_instructions(*asmw);

	hazard_t* hazard_list = (hazard_t*)malloc(sizeof(hazard_t) * len);
	int num_hazards = identify_hazards(insts, len, hazard_list, 0);

	for(int i = 0; i < len; i++){
		for(int j = num_hazards - 1; j >= 0; j--) {
			hazard_t h = hazard_list[j];

			if(h.hazard_with == i){
				if (h.type == CTRL){
					update_btype_imm(&insts[i], btype_imm(insts[i]) + 3*4);
					asmw->words[i] = insts[i].raw;
					int needs = 3;
					word_t nop = 0x00000013;

					while (asmw->len + needs > asmw->capacity) {
						asmw->capacity *= 2;
						asmw->words = (word_t*)realloc(asmw->words, sizeof(word_t) * asmw->capacity);
					}

					for(int k = asmw->len - 1; k >= i; k--){
						asmw->words[k + needs] = asmw->words[k];
					}

					for(int n = 0; n < needs; n++){
						asmw->words[i + n + 1] = nop;
					}

					asmw->len += needs;

					for (int m = 0; m < num_hazards; m++) {
						if (hazard_list[m].hazard_with > i) {
							hazard_list[m].hazard_with += needs;
						}
					}

					i += needs; 
				}
			}
		}
	}
}


void nops_on_datas(rscv_asm_words *asmw, int optimize_forwarding){
	int original_len = asmw->len;
	instruction* insts = decode_instructions(*asmw);

	hazard_t* hazard_list = (hazard_t*)malloc(sizeof(hazard_t) * original_len);
	int num_hazards = identify_hazards(insts, original_len, hazard_list, optimize_forwarding);

	word_t nop = 0x00000013;

	int* inserted_before = (int*)calloc(original_len + 50, sizeof(int));

	for (int h = 0; h < num_hazards; h++) {
		hazard_t hz = hazard_list[h];
		if (hz.type != DATA) continue;

		int producer = hz.hazard_with;
		int consumer = hz.index;

		int actual_producer_pos = producer + inserted_before[producer];
		int actual_consumer_pos = consumer + inserted_before[consumer];

		int current_gap = actual_consumer_pos - actual_producer_pos - 1;
		int required_gap = 2; 

		if (current_gap >= required_gap) {
			continue; 
		}

		int needed_nops = required_gap - current_gap;

		int insert_pos = actual_producer_pos + 1;

		while (asmw->len + needed_nops > asmw->capacity) {
			asmw->capacity *= 2;
			asmw->words = (word_t*)realloc(asmw->words, sizeof(word_t) * asmw->capacity);
		}

		for (int i = asmw->len - 1; i >= insert_pos; i--) {
			asmw->words[i + needed_nops] = asmw->words[i];
		}

		for (int i = 0; i < needed_nops; i++) {
			asmw->words[insert_pos + i] = nop;
		}

		asmw->len += needed_nops;

		for (int i = producer + 1; i < original_len; i++) {
			inserted_before[i] += needed_nops;
		}
	}
}

void handle_nops(rscv_asm_words asmw, int optimize_forwarding, int ctrl){
	if (ctrl) {
		nops_on_ctrls(&asmw);
	}
	nops_on_datas(&asmw, optimize_forwarding);
	int len = asmw.len;

	for(int i = 0; i < len; i++){
		if (asmw.words[i] == 0x00000013){
			printf("0x%08x ; nop\n", asmw.words[i]);
		} else {
			printf("0x%08x\n", asmw.words[i]);
		}
	}
}

rscv_asm_words reorder(rscv_asm_words asmw, int optimize_fowarding){
	rscv_asm_words result;
	result.len = asmw.len;
	result.capacity = asmw.capacity;
	result.words = (word_t*)malloc(sizeof(word_t) * result.capacity);
	memcpy(result.words, asmw.words, result.len * sizeof(word_t));

	nops_on_ctrls(&result);

	int len = result.len;
	instruction* insts = decode_instructions(result);

	hazard_t* hazard_list = (hazard_t*)malloc(sizeof(hazard_t) * len);
	int num_hazards = identify_hazards(insts, len, hazard_list, optimize_fowarding);

	int window = 0;
	int window_start = 0;

	for (int i = 0; i < len; i++){
		instruction current_inst = insts[i];

		if (current_inst.type == BRANCH_TYPE) {
			for (int j = 1; j <= 3 && i + j < len; j++) {
				if (result.words[i + j] != 0x00000013) continue; 

				for (int k = i - 1; k >= 0; k--) {
					instruction candidate = insts[k];
					if (candidate.type == BRANCH_TYPE) continue;
					if (is_dependent(current_inst, candidate)) continue;
					int safe_to_move = 1;
					
					// find any dependent between
					for (int x = i - 1; x > k; x--){
						instruction betw = insts[x];
						if(is_dependent(candidate, betw)) safe_to_move = 0;
						if(is_dependent(betw, candidate)) safe_to_move = 0;
					}
					if (!safe_to_move) continue;

					result.words[i + j] = result.words[k];

					for (int m = k; m < len - 1; m++) {
						result.words[m] = result.words[m + 1];
					}
					result.len--;

					insts = decode_instructions(result);  
					len = result.len;
					i--;  
					break;
				}
			}
			window = btype_imm(current_inst) / 4;
			window_start = i;
			continue;
		}
		
		int stop_at = window ? window + window_start : len;
		if (stop_at == i) window = 0;
		int start_at = (stop_at = i) ? i + 1 : window_start;

		for(int j = num_hazards - 1; j > -1; j--) {
			hazard_t h = hazard_list[j];
			if(h.index == i && h.type == DATA){
				int temp_i = i;
				int count = 0;
				for(int k = start_at; k < stop_at; k++){
					instruction candidate = insts[k];
					if (is_dependent(candidate, current_inst)) break;
					if (candidate.type == BRANCH_TYPE) break;
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

