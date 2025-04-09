#pragma once

typedef enum  {
	unknown = -1,
	BRANCH_TYPE,
	REG_TYPE,
	STORE_TYPE,
	JUMP_TYPE,
	UPP_IMM_TYPE,
	IMM_TYPE
} INSTYPES;


typedef struct {
	INSTYPES type;
} instruction;
