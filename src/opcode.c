#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opcode.h"

u32bit_t read_u32(FILE* fp, int is_little_endian) {
    u8bit_t b[4];
    if (fread(b, 1, 4, fp) != 4) return 0;

    if (is_little_endian)
        return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
    else
        return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}


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
				fprintf(stderr, "Could not find type for instruction: %08x\n, with opcode: %08b", b, opcode);
				fprintf(stderr, "Possibly passed a format not specified with -filetype or wrong encode\n");
				exit(1);
		};
}

void handle_types(const char *filename, const char * filetype, int endian) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	size_t read;

	fp = fopen(filename, "rb");
	if (fp == NULL)
			fprintf(stderr, "Could not read file %s\n", filename);


	u32bit_t instruction_bits = 0;
	if (strcmp(filetype, "bin") == 0){
		int counter = 0;
		while (!feof(fp)) {
				u32bit_t instruction_bits = read_u32(fp, endian == RSC_OBJ_LITTLE_ENDIAN);

				if (instruction_bits != 0) {
					INSTYPES type = indentify_op_type(instruction_bits);
					counter++;
					printf("Instruction %d,\t Hex Instruction: 0x%08x,\t%s\n", counter, instruction_bits, riscv_instype_str(type));
				}
		}
  } else {
			int counter = 1;
			while ((read = getline(&line, &len, fp)) != -1) {
				if (strcmp(filetype, "hexstr") == 0){
						instruction_bits = strtoull(line, (char**)0, 16);
				} else if (strcmp(filetype, "binstr") == 0){
						instruction_bits = strtoull(line, (char**)0, 2);
				} else {
					fprintf(stderr,"Unknown filetype '%s', available: [bin|hexstr|binstr]\n", filetype);
					exit(1);
				}

				INSTYPES type = indentify_op_type(instruction_bits);
				printf("Instruction %d,\t Hex Instruction: 0x%08x,\t%s\n", counter, instruction_bits, riscv_instype_str(type));
				counter++;
			}
	}

	fclose(fp);
	if (line)
			free(line);
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



