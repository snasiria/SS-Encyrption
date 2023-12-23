#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "randstate.h"
#include "numtheory.h"
#include "ss.h"

#define OPTIONS "hvb:i:n:d:s:"

int main(int argc, char **argv) {
    int opt;

    bool help = false;
    bool verbose = false;

    uint32_t bits, iters, optInd;
    char *public_key_file, *private_key_file;
    public_key_file = "ss.pub";
    private_key_file = "ss.priv";

    iters = 50;
    bits = 256;
    optInd = optind + 1;

    long seed = time(NULL);

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

        case 'b': {
            bits = atoi(argv[optInd]);
            break;
        }

        case 'i': {
            iters = atoi(argv[optInd]);
            break;
        }

        case 'n': {
            public_key_file = argv[optInd];
            break;
        }

        case 'd': {
            private_key_file = argv[optInd];
            break;
        }

        case 's': {
            seed = atoi(argv[optInd]);
            break;
        }

        default: {
            help = true;
        }
        }
        optInd = optind + 1;
    }

    // usage message
    if (help == true) {
        printf(
            "SYNOPSIS:\n   Generates an SS public/private key pair.\n\nUSAGE\n   ./keygen "
            "[hvb:i:n:d:s:]\n\nOPTIONS\n  -h\t\tDisplay program help and usage.\n  -v\t\tDisplay "
            "verbose program output.\n  -b bits\tMinimum bits needed for public key n (default: "
            "256).\n  -i iterations\tMiller-Rabin iterations for testing primes (default: 50).\n  "
            "-n pbfile\tPublic key file (default: ss.pub).\n  -d pvfile\tPrivate key file "
            "(default: ss.priv).\n  -s seed\tRandom seed for testing.\n");

        return 0;
    }

    FILE *pbfile = fopen(public_key_file, "w"); // open to write public key file
    FILE *pvfile = fopen(private_key_file, "w"); // open to write private key file
    fchmod(fileno(pvfile),
        0600); // change file permission for private key file so owner only can access

    randstate_init(seed);
    mpz_t p, q, n, d, pq;
    mpz_inits(p, q, n, d, pq, NULL);
    ss_make_pub(p, q, n, bits, iters);
    ss_make_priv(d, pq, p, q);

    char *username = getenv("USER"); // get username

    ss_write_pub(n, username, pbfile);
    ss_write_priv(pq, d, pvfile);

    if (verbose == true) {
        printf("user = %s\n", username);
        gmp_printf("p  (%d bits) = %Zd\n", mpz_sizeinbase(p, 2), p);
        gmp_printf("q  (%d bits) = %Zd\n", mpz_sizeinbase(q, 2), q);
        gmp_printf("n  (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("pq (%d bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq);
        gmp_printf("d  (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    randstate_clear();
    fclose(pbfile);
    fclose(pvfile);
    mpz_clears(p, q, n, d, pq, NULL);
    return 0;
}
