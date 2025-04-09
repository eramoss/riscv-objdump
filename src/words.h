#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "typedefs.h"

typedef struct  {
	word_t* words;
	u32bit_t len;
	u32bit_t capacity;
}rscv_asm_words;

#define RSCV_ASM_WORDS_INIT() ((rscv_asm_words){ .words = NULL, .len = 0, .capacity = 0 })

static inline int rscv_asm_words_grow(rscv_asm_words* asmw, u32bit_t min_capacity) {
	u32bit_t new_capacity = asmw->capacity ? asmw->capacity * 2 : 4;
	if (new_capacity < min_capacity)
			new_capacity = min_capacity;

	word_t* new_words = (word_t*)realloc(asmw->words, new_capacity * sizeof(word_t));
	if (!new_words) return -1;

	asmw->words = new_words;
	asmw->capacity = new_capacity;
	return 0;
}

static inline int rscv_asm_words_append(rscv_asm_words* asmw, word_t value) {
	if (asmw->len >= asmw->capacity) {
			if (rscv_asm_words_grow(asmw, asmw->len + 1) != 0)
					return -1;
	}
	asmw->words[asmw->len++] = value;
	return 0;
}

static inline void rscv_asm_words_free(rscv_asm_words* asmw) {
	free(asmw->words);
	asmw->words = NULL;
	asmw->len = 0;
	asmw->capacity = 0;
}

static inline void rscv_asm_words_clear(rscv_asm_words* asmw) {
  asmw->len = 0;
}


static inline int read_word(FILE *fp, int little_endian, word_t *out_word) {
	u8bit_t buffer[4];
	if (fread(buffer, 1, 4, fp) != 4) {
			return 0;
	}
	if (little_endian) {
			*out_word = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
	} else {
			*out_word = buffer[3] | (buffer[2] << 8) | (buffer[1] << 16) | (buffer[0] << 24);
	}

	return 1;
}


static inline rscv_asm_words asmw_from_asm(const char * filename, const char* filetype, int endian){
	rscv_asm_words asmw = RSCV_ASM_WORDS_INIT();
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	size_t read;
	fp = fopen(filename, "rb");
	if (fp == NULL){
			fprintf(stderr, "Could not read file %s\n", filename);
			exit(1);
	}

	word_t instruction_bits = 0;
	if (strcmp(filetype, "bin") == 0){
		while (read_word(fp, endian == RSC_OBJ_LITTLE_ENDIAN, &instruction_bits)) {
				rscv_asm_words_append(&asmw, instruction_bits);
		}
  } else {
			while ((read = getline(&line, &len, fp)) != -1) {
				if (strcmp(filetype, "hexstr") == 0){
						instruction_bits = strtoull(line, (char**)0, 16);
				} else if (strcmp(filetype, "binstr") == 0){
						instruction_bits = strtoull(line, (char**)0, 2);
				} else {
					fprintf(stderr,"Unknown filetype '%s', available: [bin|hexstr|binstr]\n", filetype);
					exit(1);
				}
				rscv_asm_words_append(&asmw, instruction_bits);
			}
	}

	fclose(fp);
	if (line)
			free(line);

	return asmw;
}
