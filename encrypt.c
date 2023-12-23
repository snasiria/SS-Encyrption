#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>
#include "randstate.h"
#include "numtheory.h"
#include "ss.h"

#define OPTIONS "hvi:o:n:"

int main(int argc, char **argv) {
    int opt;
    bool verbose = false;
    bool help = false;

    char *input_file, *output_file, *public_key_file;
    input_file = NULL;
    output_file = NULL;
    public_key_file = "ss.pub";

    int optInd = optind + 1;

    // manages user inputs
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'v': {
            verbose = true;
            break;
        }

        case 'h': {
            help = true;
            break;
        }

        case 'i': {
            input_file = argv[optInd];
            break;
        }

        case 'o': {
            output_file = argv[optInd];
            break;
        }

        case 'n': {
            public_key_file = argv[optInd];
            break;
        }

        default: {
            help = true;
            break;
        }
        }
        optInd = optind + 1;
    }

    // usage message
    if (help == true) {
        printf(
            "SYNOPSIS:\n   Encrypts data using an SS encryption.\n   Encrypted data is "
            "decrypted by the decrypt program.\n\nUSAGE\n   ./encrypt [hvi:o:n:]\n\nOPTIONS\n  "
            "-h\t\t\tDisplay program help and usage.\n  -v\t\t\tDisplay verbose program "
            "output.\n  -i infile\t\tInput file of data to encrypt (default: stdin).\n  -o "
            "outfile\t\tOutput file for encrypted data (default: stdout).\n  -n pbfile\t\tPublic "
            "key file (default: ss.pub).\n");
        return 0;
    }

    // file containing message
    FILE *infile = stdin;

    // if input file provided, use that instead of stdin
    if (input_file != NULL) {
        infile = fopen(input_file, "r"); // open to read

        // if file doesn't exist in directory
        if (infile == NULL) {
            printf("%s: No such file or directory\n", input_file);
            return 0;
        }
    }

    // file that will contain encrypted ciphertext
    FILE *outfile = stdout;

    // if output file provided, use that instead of stdout
    if (output_file != NULL) {
        outfile = fopen(output_file, "w"); // open to write

        // if file doesn't exist in directory
        if (outfile == NULL) {
            printf("%s: No such file or directory\n", output_file);
            return 0;
        }
    }

    // file that contains public key
    FILE *pbfile = fopen(public_key_file, "r");

    if (pbfile == NULL) {
        printf("%s: No such file or directory\n", public_key_file);
        return 0;
    }

    mpz_t n;
    mpz_init(n);
    char username[_POSIX_LOGIN_NAME_MAX]; //maximum possible username
    ss_read_pub(n, username, pbfile);
    fclose(pbfile);

    if (verbose == true) {
        printf("user = %s\n", username);
        gmp_printf("n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
    }

    ss_encrypt_file(infile, outfile, n);
    fclose(infile);
    fclose(outfile);
    mpz_clear(n);
    return 0;
}
