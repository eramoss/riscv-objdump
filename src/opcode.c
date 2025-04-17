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
				fprintf(stderr, "Possibly passed a format not specified with --type or wrong encode\n");
				exit(1);
		};
}

instruction* decode_instructions(rscv_asm_words asmw){
	int len = asmw.len;
	instruction * insts = (instruction*)malloc(len * sizeof(instruction));

	for (int i = 0; i < len; i ++ ){
		instruction inst;
		word_t raw = asmw.words[i];
		INSTYPES type = indentify_op_type(raw);
		inst.raw = raw;
		inst.type = type;
		insts[i] = inst;
	}

	return insts;
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

void print_instruction(instruction inst) {
    printf("Instruction Type: ");
    switch (inst.type) {
        case REG_TYPE:
            printf("R-Type\n");
            printf("  opcode : 0x%02X\n", inst.r.opcode);
            printf("  rd     : x%d\n", inst.r.rd);
            printf("  funct3 : 0x%X\n", inst.r.funct3);
            printf("  rs1    : x%d\n", inst.r.rs1);
            printf("  rs2    : x%d\n", inst.r.rs2);
            printf("  funct7 : 0x%X\n", inst.r.funct7);
            break;

        case IMM_TYPE:
            printf("I-Type\n");
            printf("  opcode : 0x%02X\n", inst.i.opcode);
            printf("  rd     : x%d\n", inst.i.rd);
            printf("  funct3 : 0x%X\n", inst.i.funct3);
            printf("  rs1    : x%d\n", inst.i.rs1);
            printf("  imm    : %d\n", inst.i.imm);  // Sign-extended if needed
            break;

        case STORE_TYPE:
            printf("S-Type\n");
            printf("  opcode : 0x%02X\n", inst.s.opcode);
            printf("  rs1    : x%d\n", inst.s.rs1);
            printf("  rs2    : x%d\n", inst.s.rs2);
            printf("  funct3 : 0x%X\n", inst.s.funct3);
            printf("  imm    : %d\n", (inst.s.imm_5_11 << 5) | inst.s.imm_0_4);
            break;

        case BRANCH_TYPE:
            printf("B-Type\n");
            printf("  opcode : 0x%02X\n", inst.b.opcode);
            printf("  rs1    : x%d\n", inst.b.rs1);
            printf("  rs2    : x%d\n", inst.b.rs2);
            printf("  funct3 : 0x%X\n", inst.b.funct3);
            printf("  imm    : %d\n", 
                (inst.b.imm_12 << 12) | 
                (inst.b.imm_11 << 11) |
                (inst.b.imm_5_10 << 5) |
                (inst.b.imm_1_4 << 1));
            break;

        case UPP_IMM_TYPE:
            printf("U-Type\n");
            printf("  opcode : 0x%02X\n", inst.u.opcode);
            printf("  rd     : x%d\n", inst.u.rd);
            printf("  imm    : %d\n", inst.u.imm << 12);
            break;

        case JUMP_TYPE:
            printf("J-Type\n");
            printf("  opcode : 0x%02X\n", inst.j.opcode);
            printf("  rd     : x%d\n", inst.j.rd);
            printf("  imm    : %d\n", 
                (inst.j.imm_20 << 20) |
                (inst.j.imm_12_19 << 12) |
                (inst.j.imm_11 << 11) |
                (inst.j.imm_1_10 << 1));
            break;

        default:
            printf("Unknown type\n");
            printf("  Raw: 0x%08X\n", inst.raw);
    }
}


void handle_types(rscv_asm_words asmw) {
	int len = asmw.len;
	for (int i = 0; i < len; i ++ ){
		INSTYPES type = indentify_op_type(asmw.words[i]);
		printf("Instruction %d,\t Hex Instruction: 0x%08x,\t%s\n", i+1, asmw.words[i], riscv_instype_str(type));
	}
}

void handle_decode(rscv_asm_words asmw){
	int len = asmw.len;
	instruction* insts = decode_instructions(asmw);

	for (int i = 0; i < len; i ++ ){
		print_instruction(insts[i]);
	}
}



