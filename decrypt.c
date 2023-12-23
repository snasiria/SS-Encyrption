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

    char *input_file, *output_file, *private_key_file;
    input_file = NULL;
    output_file = NULL;
    private_key_file = "ss.priv";

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
            private_key_file = argv[optInd];
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
        printf("SYNOPSIS:\n   Decrypts data using an SS encryption.\n   Encrypted data is "
               "encrypted by the encrypted program.\n\nUSAGE\n   ./decrypt [hvi:o:n:]\n\nOPTIONS\n "
               " -h\t\t\tDisplay program help and usage.\n  -v\t\t\tDisplay verbose program "
               "output.\n  -i infile\t\tInput file of data to decrypt (default: stdin).\n  -o "
               "outfile\t\tOutput file for decrypted data (default: stdout).\n  -n "
               "pbfile\t\tPrivate key file (default: ss.priv).\n");
        return 0;
    }

    FILE *infile = stdin; // file containing encrypted ciphertext

    // if input file provided, use that instead of stdin
    if (input_file != NULL) {
        infile = fopen(input_file, "r");

        // if file doesn't exist in directory
        if (infile == NULL) {
            printf("%s: No such file or directory\n", input_file);
            return 0;
        }
    }

    FILE *outfile = stdout; // file that will contain decrypted plaintext

    // if output file provided, use that instead of stdout
    if (output_file != NULL) {
        outfile = fopen(output_file, "w"); // open file to write

        // if file doesn't exist in directory
        if (outfile == NULL) {
            printf("%s: No such file or directory\n", output_file);
            return 0;
        }
    }

    FILE *pvfile = fopen(private_key_file, "r"); // read file containing private key

    if (private_key_file == NULL) {
        printf("%s: No such file or directory\n", private_key_file);
        return 0;
    }

    mpz_t d, pq;
    mpz_inits(d, pq, NULL);
    ss_read_priv(pq, d, pvfile);
    fclose(pvfile);

    if (verbose == true) {
        gmp_printf("pq (%d bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq);
        gmp_printf("d  (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    ss_decrypt_file(infile, outfile, d, pq);
    fclose(infile);
    fclose(outfile);
    mpz_clears(d, pq, NULL);
    return 0;
}
