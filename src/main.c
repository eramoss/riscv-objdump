#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "opcode.h"
#include "typedefs.h"

#define ARG_IS(s) (strcmp(arg, s) == 0)
#define ARG_HAS_PREFIX(p) (strncmp(arg, p, strlen(p)) == 0)
#define ARG_VALUE(p) (arg + strlen(p))

void usage(const char *progname) {
    printf("Usage:\n%s [commands] [options]\n", progname);
    printf("Commands:\n");
    printf("\t-types          Run types handler\n");
    printf("\t-decode         Run decode handler\n");
    printf("Options:\n");
    printf("\t--file=<filename>     Specify the input file\n");
    printf("\t--endian=little|big  (optional, used by -types)\n");
    printf("\t-filetype=<type>     (optional, used by -types) [bin | hexstr | binstr] default: hexstr\n");
}

int parse_endian(const char *value) {
    if (strcmp(value, "little") == 0) return RSC_OBJ_LITTLE_ENDIAN;
    if (strcmp(value, "big") == 0) return RSC_OBJ_BIG_ENDIAN;
    fprintf(stderr, "Invalid endian value: %s\n", value);
    exit(1);
}

typedef struct {
    const char *filename;
    const char *filetype;
    int endian;
} ProgramOptions;

typedef struct {
    int run_types;
    int run_decode;
} CommandFlags;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    ProgramOptions opts = {
        .filename = NULL,
        .filetype = "hexstr",
        .endian = RSC_OBJ_LITTLE_ENDIAN
    };

    CommandFlags cmds = {0};

    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (ARG_IS("-types")) {
            cmds.run_types = 1;
        } else if (ARG_IS("-decode")) {
            cmds.run_decode = 1;
        } else if (ARG_HAS_PREFIX("--file=")) {
            opts.filename = ARG_VALUE("--file=");
        } else if (ARG_HAS_PREFIX("--endian=")) {
            opts.endian = parse_endian(ARG_VALUE("--endian="));
        } else if (ARG_HAS_PREFIX("-filetype=")) {
            opts.filetype = ARG_VALUE("-filetype=");
        } else {
            fprintf(stderr, "Unknown option: %s\n", arg);
            usage(argv[0]);
            return 1;
        }
    }

    if (!opts.filename) {
        fprintf(stderr, "Error: --file is required.\n");
        return 1;
    }

    if (cmds.run_types) {
        handle_types(opts.filename, opts.filetype, opts.endian);
    }

    if (cmds.run_decode) {
    }

    return 0;
}
