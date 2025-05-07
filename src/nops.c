#include "nops.h"

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


void handle_nops(rscv_asm_words asmw, int optimize_forwarding){
	int len = asmw.len;
	instruction* insts = decode_instructions(asmw);

	hazard_t* hazard_list = (hazard_t*)malloc(sizeof(hazard_t) * len);
	int num_hazards = identify_hazards(insts, len, hazard_list);
	
	for(int i = 0; i < len; i++){
		printf("0x%08x\n", asmw.words[i]);
		for(int j= num_hazards-1; j > -1; j--) {
			hazard_t h = hazard_list[j];

			if (optimize_forwarding && h.type == DATA) {
					instruction producer = insts[h.hazard_with];
					instruction consumer = insts[h.index];

					if (can_be_forwarded(producer, consumer)) {
							continue;
					}
			}

			if(h.hazard_with == i){
				for(int num_nops = 0; num_nops < (h.index - h.hazard_with); num_nops++)
					printf("0x00000013  ; nop\n");
				break;
			}
		}
	}
}



