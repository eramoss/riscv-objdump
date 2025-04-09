#include "opcode.h"

INSTYPES indentify_op_type(u32bit_t b){
	int opcode = 0x7F & b;
	switch (opcode) {
			case FEN_I_TYPE: case OP_I_TYPE: case LI_TYPE: case I_TYPE:
				return IMM_TYPE;
			case LUI: case AUIPC: 
				return UPP_IMM_TYPE;
			case J_TYPE:
				return JUMP_TYPE;
			case R_TYPE:
				return REG_TYPE;
			case B_TYPE:
				return BRANCH_TYPE;
			case S_TYPE:
				return STORE_TYPE;
			default:
				fprintf(stderr, "Could not find type for instruction: 0x%08x, with opcode: 0b%08b\n", b, opcode);
				fprintf(stderr, "Possibly passed a format not specified with -filetype or wrong encode\n");
				exit(1);
		};
}

void handle_types(rscv_asm_words asmw) {
	int len = asmw.len;
	for (int i = 0; i < len; i ++ ){
		INSTYPES type = indentify_op_type(asmw.words[i]);
		printf("Instruction %d,\t Hex Instruction: 0x%08x,\t%s\n", i+1, asmw.words[i], riscv_instype_str(type));
	}
}

const char* riscv_instype_str(INSTYPES type) {
	switch (type) {
		case BRANCH_TYPE:   return "B-type (Branch)";
		case REG_TYPE:      return "R-type (Register)";
		case STORE_TYPE:    return "S-type (Store)";
		case JUMP_TYPE:     return "J-type (Jump)";
		case UPP_IMM_TYPE:  return "U-type (Upper Immediate)";
		case IMM_TYPE:      return "I-type (Immediate)";
		case unknown:
		default:            return "Unknown Type";
	}
}



