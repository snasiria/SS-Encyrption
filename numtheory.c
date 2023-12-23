#include "randstate.h"
#include <gmp.h>
#include "numtheory.h"

// puts greatest common denominator of a and b into g
void gcd(mpz_t g, const mpz_t a, const mpz_t b) {
    mpz_t t, a_copy, b_copy;
    mpz_inits(t, a_copy, b_copy, NULL);

    mpz_set(a_copy, a);
    mpz_set(b_copy, b);

    while (mpz_cmp_ui(b_copy, 0) != 0) {
        mpz_set(t, b_copy);
        mpz_mod(b_copy, a_copy, b_copy);
        mpz_set(a_copy, t);
    }

    mpz_set(g, a_copy);
    mpz_clears(t, a_copy, b_copy, NULL);
}

// finds modular inverse of a and n
void mod_inverse(mpz_t o, const mpz_t a, const mpz_t n) {
    mpz_t remainder, remainder_prime, s, s_prime, t, t_prime, q;
    mpz_inits(remainder, remainder_prime, s, s_prime, t, t_prime, q, NULL);

    mpz_set(remainder, n);
    mpz_set(remainder_prime, a);
    mpz_set_ui(s, 1);
    mpz_set_ui(s_prime, 0);
    mpz_set_ui(t, 0);
    mpz_set_ui(t_prime, 1);

    while (mpz_cmp_ui(remainder_prime, 0) != 0) {
        mpz_fdiv_q(q, remainder, remainder_prime); // q = (remainder / remainder_prime)

        // store current values into a placeholder temporary variable
        mpz_t a_temp, s_temp, t_temp;
        mpz_inits(a_temp, s_temp, t_temp, NULL);
        mpz_set(a_temp, remainder);
        mpz_set(s_temp, s);
        mpz_set(t_temp, t);

        mpz_set(remainder, remainder_prime); // remainder = remainder_prime
        mpz_mul(remainder_prime, q, remainder_prime); //remainder_prime = q * remainder_prime
        mpz_sub(
            remainder_prime, a_temp, remainder_prime); // remainder_prime = a_temp - remainder_prime

        mpz_set(s, s_prime); // s = s_prime
        mpz_mul(s_prime, q, s_prime); //s_prime = q * s_prime
        mpz_sub(s_prime, s_temp, s_prime); // s_prime = s_temp - s_prime

        mpz_set(t, t_prime); // t = t_prime
        mpz_mul(t_prime, q, t_prime); //t_prime = q * t_prime
        mpz_sub(t_prime, t_temp, t_prime); // t_prime = t_temp - t_prime

        mpz_clears(a_temp, s_temp, t_temp, NULL);
    }

    if (mpz_cmp_ui(remainder, 1) > 0) {
        mpz_set_ui(o, 0);
        mpz_clears(remainder, remainder_prime, s, s_prime, t, t_prime, q, NULL);
        return;
    }

    // if negative, subtract from n
    if (mpz_cmp_ui(t, 0) < 0)
        mpz_add(t, t, n);

    mpz_set(o, t);
    mpz_clears(remainder, remainder_prime, s, s_prime, t, t_prime, q, NULL);
}

// sets 'o' to the result of a^d mod n
void pow_mod(mpz_t o, const mpz_t a, const mpz_t d, const mpz_t n) {
    mpz_t v, p, i;
    mpz_inits(v, p, i, NULL);
    mpz_set_ui(v, 1);
    mpz_set(p, a);
    mpz_set(i, d);

    while ((mpz_cmp_ui(i, 0) > 0)) {
        if (mpz_odd_p(i)) {
            mpz_mul(v, v, p);
            mpz_mod(v, v, n);
        }
        mpz_mul(p, p, p);
        mpz_mod(p, p, n);
        mpz_fdiv_q_ui(i, i, 2);
    }

    mpz_set(o, v);
    mpz_clears(v, p, i, NULL);
}

// determines whether n is likely prime (true) or not (false)
bool is_prime(const mpz_t n, uint64_t iters) {
    // 0 and 1 are special cases which are neither prime nor composite, but we still want to return false
    if (mpz_cmp_ui(n, 0) == 0 || mpz_cmp_ui(n, 1) == 0)
        return false;

    // 2 and 3 are a special cases where it is prime, but it will not work with miller-rabin algorithm
    if (mpz_cmp_ui(n, 2) == 0 || mpz_cmp_ui(n, 3) == 0)
        return true;

    // n_copy will be a copy of (n-1) to find out k
    mpz_t n_copy, k, m;
    mpz_inits(n_copy, k, m, NULL);

    mpz_set(n_copy, n);
    mpz_sub_ui(n_copy, n_copy, 1);

    // if (n-1) is even:
    while (mpz_even_p(n_copy)) {
        mpz_div_ui(n_copy, n_copy, 2);
        mpz_add_ui(k, k, 1);
    }

    mpz_set(m, n_copy);
    mpz_set(n_copy, n);
    mpz_sub_ui(n_copy, n, 3);

    mpz_t n_minus_one;
    mpz_init(n_minus_one);
    mpz_set(n_minus_one, n);
    mpz_sub_ui(n_minus_one, n, 1);

    // set k to k-1
    mpz_sub_ui(k, k, 1);

    // step two
    mpz_t a;
    mpz_init(a);

    for (uint64_t i = 0; i < iters; i++) {
        mpz_urandomm(a, state, n_copy);
        mpz_add_ui(a, a, 2);

        mpz_t y;
        mpz_init(y);
        pow_mod(y, a, m, n);

        if (mpz_cmp_ui(y, 1) != 0 && mpz_cmp(y, n_minus_one) != 0) {

            mpz_t j;
            mpz_init(j);
            mpz_set_ui(j, 1);

            while (mpz_cmp(j, k) <= 0 && mpz_cmp(y, n_minus_one) != 0) {
                mpz_t two;
                mpz_init(two);
                mpz_set_ui(two, 2);
                pow_mod(y, y, two, n);

                if (mpz_cmp_ui(y, 1) == 0) {
                    mpz_clears(n_copy, k, m, n_minus_one, a, y, j, two, NULL);
                    return false;
                }

                mpz_add_ui(j, j, 1);
                mpz_clear(two);
            }

            // if y is not equal to (n-1) -- if it's composite
            if (mpz_cmp(y, n_minus_one) != 0) {
                mpz_clears(n_copy, k, m, n_minus_one, a, y, j, NULL);
                return false;
            }
            mpz_clear(j);
        }

        mpz_clear(y);
    }

    mpz_clears(n_copy, k, m, n_minus_one, a, NULL);
    return true;
}

// make a prime number at least *bits* number of bits
void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    // addition will make sure range is between 2^(n-1) inclusive and 2^n exclusive
    mpz_t addition;
    mpz_init(addition);
    bool primeSatisfied = false;

    // because 1 bit offers no prime numbers, we will allow for an extra bit
    if (bits == 1)
        mpz_ui_pow_ui(addition, 2, bits);
    else
        mpz_ui_pow_ui(addition, 2, bits - 1);

    while (primeSatisfied == false) {
        mpz_urandomb(p, state, bits);
        mpz_add(p, p, addition);

        if (is_prime(p, iters) == true)
            primeSatisfied = true;
    }

    mpz_clear(addition);
}
