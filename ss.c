#include "ss.h"
#include "numtheory.h"
#include "randstate.h"
#include <stdlib.h>
#include <gmp.h>

// makes a public key
void ss_make_pub(mpz_t p, mpz_t q, mpz_t n, uint64_t nbits, uint64_t iters) {

    mpz_t log_condition, larger_val, smaller_val, mod_value, n_bits;

    bool isNotDivisible;

    mpz_inits(log_condition, larger_val, smaller_val, mod_value, n_bits, NULL);
    mpz_set_ui(n_bits, nbits);

    // loop to make p, q, n values until proper conditions are met
    do {
        isNotDivisible = true; //true by default
        uint64_t upper_bound = ((2 * nbits) / 5);
        uint64_t lower_bound = (nbits / 5);

        // generate p_bits, q_bits
        uint64_t p_bits = (random() % (upper_bound - lower_bound) + lower_bound);
        uint64_t q_bits = (nbits - (2 * p_bits));

        make_prime(p, p_bits, iters);
        make_prime(q, q_bits, iters);

        //generate n_bits based on p and q
        mpz_mul(n, p, p);
        mpz_mul(n, n, q);

        // calculates whether p or q is larger or smaller, used for modular arithmetic for condition 1
        if (mpz_cmp(p, q) > 0) {
            mpz_set(larger_val, p);
            mpz_set(smaller_val, q);
        } else {
            mpz_set(larger_val, q);
            mpz_set(smaller_val, p);
        }

        mpz_sub_ui(larger_val, larger_val, 1);
        mpz_mod(mod_value, larger_val, smaller_val);

        // condition 1: if (p-1) is not divisible with q and (q-1) is not divisible with p
        if (mpz_cmp_ui(mod_value, 0) == 0)
            isNotDivisible = false;
        mpz_add_ui(larger_val, larger_val, 1);

        mpz_sub_ui(smaller_val, smaller_val, 1);
        mpz_mod(mod_value, larger_val, smaller_val);
        if (mpz_cmp_ui(mod_value, 0) == 0)
            isNotDivisible = false;

        // condition 2: calculates integer of log2(n), later checks if log2(n) >= nbits
        size_t log = mpz_sizeinbase(n, 2);
        log = log - 1;
        mpz_set_ui(log_condition, log);

    } while (mpz_cmp(log_condition, n_bits) < 0
             || isNotDivisible == false); //makes sure those 2 conditions are satisfied

    mpz_clears(log_condition, larger_val, smaller_val, mod_value, n_bits, NULL);
}

// calculates least common multiple of a and b, stores it in r
void lcm(mpz_t r, mpz_t a, mpz_t b) {
    mpz_t gcd_for_lcm;
    mpz_init(gcd_for_lcm);
    gcd(gcd_for_lcm, a, b);

    mpz_mul(r, a, b);
    mpz_fdiv_q(r, r, gcd_for_lcm); // (a * b) / (gcd(a,b))

    mpz_clear(gcd_for_lcm);
}

// generates private key
void ss_make_priv(mpz_t d, mpz_t pq, const mpz_t p, const mpz_t q) {
    mpz_t mod, p_minus_1, q_minus_1, n;
    mpz_inits(mod, p_minus_1, q_minus_1, n, NULL);

    mpz_set(p_minus_1, p);
    mpz_set(q_minus_1, q);
    mpz_sub_ui(p_minus_1, p_minus_1, 1);
    mpz_sub_ui(q_minus_1, q_minus_1, 1);

    mpz_mul(pq, p, q);

    mpz_mul(n, p, p);
    mpz_mul(n, n, q);

    lcm(mod, p_minus_1, q_minus_1);
    mod_inverse(d, n, mod);

    mpz_clears(mod, p_minus_1, q_minus_1, n, NULL);
}

// writes public key to file
void ss_write_pub(const mpz_t n, const char username[], FILE *pbfile) {
    gmp_fprintf(pbfile, "%Zx\n", n);
    gmp_fprintf(pbfile, username);
}

// writes private key to file
void ss_write_priv(const mpz_t pq, const mpz_t d, FILE *pvfile) {
    gmp_fprintf(pvfile, "%Zx\n", pq);
    gmp_fprintf(pvfile, "%Zx", d);
}

// reads public key from file
void ss_read_pub(mpz_t n, char username[], FILE *pbfile) {
    gmp_fscanf(pbfile, "%Zx %s", n, username);
}

// reads private key from file
void ss_read_priv(mpz_t pq, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%Zx %Zx", pq, d);
}

// generate ciphertext
void ss_encrypt(mpz_t c, const mpz_t m, const mpz_t n) {
    pow_mod(c, m, n, n);
}

// encrypts plaintext from infile to outfile
void ss_encrypt_file(FILE *infile, FILE *outfile, const mpz_t n) {
    // calculates log 2(n)
    size_t k = mpz_sizeinbase(n, 2);
    k /= 2;
    k -= 2;
    k /= 8;

    uint8_t *block_arr = (uint8_t *) calloc(k, sizeof(uint8_t));

    unsigned long j;
    mpz_t m, c;
    mpz_inits(m, c, NULL);

    block_arr[0] = 0xFF;

    do {
        j = fread(block_arr + 1, sizeof(uint8_t), k - 1, infile);

        // if the number of bytes left is less than k-1 or it hits 0, in other words we're coming to an end
        if (j < (k - 1) || j == 0) {
            block_arr[j + 1]
                = 0; //makes sure to cut off anything after j in case it's less than k-1
            mpz_import(
                m, k, sizeof(uint8_t), 1, 1, 0, block_arr); // fills block_arr with binary data
            ss_encrypt(c, m, n);
            gmp_fprintf(outfile, "%Zx\n", c); // print to export file
            break; // exit from loop
        }

        mpz_import(m, k, sizeof(uint8_t), 1, 1, 0, block_arr); // fills block_arr with binary data
        ss_encrypt(c, m, n);
        gmp_fprintf(outfile, "%Zx\n", c);

    } while (true);

    free(block_arr);
    mpz_clears(m, c, NULL);
}

// decrypt ciphertext
void ss_decrypt(mpz_t m, const mpz_t c, const mpz_t d, const mpz_t pq) {
    pow_mod(m, c, d, pq);
}

// decrypt ciphertext from infile to outfile
void ss_decrypt_file(FILE *infile, FILE *outfile, const mpz_t d, const mpz_t pq) {
    size_t k = mpz_sizeinbase(pq, 2);
    k -= 1;
    k /= 8;

    uint8_t *block_arr = (uint8_t *) calloc(k, sizeof(uint8_t));

    mpz_t c, m;
    mpz_inits(c, m, NULL);

    // iterate over each line block
    do {
        int res = gmp_fscanf(infile, "%Zx", c); // scan hexstring from that line

        if (res == EOF) // if we reached end of file
            break; //exit from loop

        ss_decrypt(m, c, d, pq);
        size_t j;
        mpz_export(block_arr + 1, &j, 1, sizeof(uint8_t), 1, 0,
            m); // export to binary data, starting at index 1
        fprintf(outfile, "%s", &block_arr[2]); // output after the prepended byte
    } while (true);

    free(block_arr);
    mpz_clears(c, m, NULL);
}
