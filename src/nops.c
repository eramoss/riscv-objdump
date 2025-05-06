#include "nops.h"

void handle_nops(rscv_asm_words asmw, int opt){
	int len = asmw.len;
	instruction* insts = decode_instructions(asmw);

	hazard_t* hazard_list = (hazard_t*)malloc(sizeof(hazard_t) * len);
	int num_hazards = identify_hazards(insts, len, hazard_list);
	
	int last_hazard = -1;
	for(int i = 0; i < len; i++){
		printf("0x%08x\n", asmw.words[i]);
		for(int j= 0; j< num_hazards; j++) {
			if(last_hazard != i && hazard_list[j].hazard_with == i){
				last_hazard = i;
				if (hazard_list[j].type == CTRL){
					for(int num_nops = 0; num_nops <3; num_nops++)
						printf("0x00000013  ; auto generated\n");
					break;
				}
				else if (hazard_list[j].type == DATA){
					for(int num_nops = 0; num_nops <2; num_nops++)
						printf("0x00000013  ; auto generated\n");
					break;
				}
			}
		}
	}
}
