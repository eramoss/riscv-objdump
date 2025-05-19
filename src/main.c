#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "opcode.h"
#include "typedefs.h"
#include "words.h"
#include "hazards.h"

#define ARG_IS(s) (strcmp(arg, s) == 0)
#define ARG_HAS_PREFIX(p) (strncmp(arg, p, strlen(p)) == 0)
#define ARG_VALUE(p) (arg + strlen(p))

void usage(const char *progname) {
    printf("Usage:\n%s [commands] [options]\n", progname);
    printf("Commands:\n");
    printf("\t-types               Types of each instruction in post compiled riscv asm\n");
    printf("\t-decode              Decode each instruction into its fields\n");
    printf("\t-hazards             Print data hazards\n");
    printf("\t-nops                Insert Nops in hex\n");
    printf("\nArgs:\n");
    printf("\t-file <filename>     <REQUIRED> Specify the input file\n"); 
    printf("\nOptions:\n");
    printf("\t--type=<type>        The format of the file passed [bin | hexstr | binstr] default: hexstr\n");
    printf("\t--endian=little|big  The endianess of the file passed (when `bin` in type) default: little\n");
    printf("\t-OF=1|0             Activate optimizing fowarding for insert Nops         default: 0\n");
}

int parse_endian(const char *value) {
    if (strcmp(value, "little") == 0) return RSC_OBJ_LITTLE_ENDIAN;
    if (strcmp(value, "big") == 0) return RSC_OBJ_BIG_ENDIAN;
    fprintf(stderr, "Invalid endian value: %s\n", value);
    exit(1);
}

typedef struct {
    const char *filetype;
    int endian;
		int optmize_fowarding;
} ProgramOptions;

typedef struct {
    const char *filename;
} ProgramArgs;

typedef struct {
    int run_types;
    int run_decode;
		int run_hazards;
		int run_nops;
		int run_reorder;
} CommandFlags;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    ProgramOptions opts = {
        .filetype = "hexstr",
        .endian = RSC_OBJ_LITTLE_ENDIAN,
				.optmize_fowarding = 0
    };

		ProgramArgs args = {
        .filename = NULL,
		};	

    CommandFlags cmds = {0};

    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (ARG_IS("-types")) {
            cmds.run_types = 1;
        } else if (ARG_IS("-decode")) {
            cmds.run_decode = 1;
        } else if (ARG_IS("-hazards")) {
            cmds.run_hazards = 1;
        } else if (ARG_IS("-nops")) {
						cmds.run_nops = 1;
        } else if (ARG_IS("-reorder")) {
						cmds.run_reorder = 1;
        } else if (ARG_IS("-file")) {
            args.filename = argv[++i];
        } else if (ARG_HAS_PREFIX("--endian=")) {
            opts.endian = parse_endian(ARG_VALUE("--endian="));
        } else if (ARG_HAS_PREFIX("--type=")) {
            opts.filetype = ARG_VALUE("--type=");
        } else if (ARG_HAS_PREFIX("-OF=")) {
						opts.optmize_fowarding = atoi(ARG_VALUE("-OF="));
        } else {
            fprintf(stderr, "Unknown option: %s\n", arg);
            usage(argv[0]);
            return 1;
        }
    }

    if (!args.filename) {
        fprintf(stderr, "Error: -file is required.\n");
        return 1;
    }
		rscv_asm_words asmw = asmw_from_asm(args.filename, opts.filetype, opts.endian);

    if (cmds.run_types) {
        handle_types(asmw);
    }

    if (cmds.run_decode) {
				handle_decode(asmw);
    }

    if (cmds.run_hazards) {
				handle_hazards(asmw,opts.optmize_fowarding);
    }
		
    if (cmds.run_nops) {
				handle_nops(asmw, opts.optmize_fowarding, 1);
    }
	  
		if (cmds.run_reorder) {
				rscv_asm_words new_asm = reorder(asmw, opts.optmize_fowarding);
				handle_nops(new_asm, opts.optmize_fowarding, 0);
    }


    return 0;
}
