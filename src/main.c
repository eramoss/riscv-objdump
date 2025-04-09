#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "opcode.h"
#include "typedefs.h"


void usage(const char *progname) {
    printf("Usage:\n%s\n", progname);
    printf("\t-types <filename> [filetype]\n");
    printf("\t\tAvailable filetypes: bin [--endian=little|big] | hexstr | binstr (default: hexstr)\n");
}

int parse_endian(const char *arg) {
    if (strcmp(arg, "little") == 0) return RSC_OBJ_LITTLE_ENDIAN;
    if (strcmp(arg, "big") == 0) return RSC_OBJ_BIG_ENDIAN;
    fprintf(stderr, "Invalid endian value: %s\n", arg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "-types") == 0) {
        const char *filename = NULL;
        const char *filetype = "hexstr"; 
        int endian = RSC_OBJ_LITTLE_ENDIAN;     

        for (int i = 2; i < argc; ++i) {
            if (strncmp(argv[i], "--endian=", 9) == 0) {
                endian = parse_endian(argv[i] + 9);
            } else if (!filename) {
                filename = argv[i];
            } else {
                filetype = argv[i];
            }
        }

        if (!filename) {
            fprintf(stderr, "Error: Missing filename for -types.\n");
            usage(argv[0]);
            return 1;
        }

        handle_types(filename, filetype, endian);
    }


    else {
        fprintf(stderr, "Unknown command: %s\n", command);
        usage(argv[0]);
        return 1;
    }

    return 0;
}
