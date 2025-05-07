#include "nops.h"

void handle_nops(rscv_asm_words asmw, int opt){
	int len = asmw.len;
	instruction* insts = decode_instructions(asmw);

	hazard_t* hazard_list = (hazard_t*)malloc(sizeof(hazard_t) * len);
	int num_hazards = identify_hazards(insts, len, hazard_list);
	
	for(int i = 0; i < len; i++){
		printf("0x%08x\n", asmw.words[i]);
		for(int j= num_hazards-1; j > -1; j--) {
			hazard_t h = hazard_list[j];
			if(h.hazard_with == i){
				for(int num_nops = 0; num_nops < (h.index - h.hazard_with); num_nops++)
					printf("0x00000013  ; nop\n");
				break;
			}
		}
	}
}
