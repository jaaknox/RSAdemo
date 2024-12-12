#include "numtheory.h"
#include "randstate.h"
#include <gmp.h>

gmp_randstate_t state;

// Function to calculate the greatest common denominator
// of two mpz_t's, a and b, and place the result in the
// mpz_t d.
//
// The function returns nothing, just passes out the gcd.
void gcd(mpz_t d, mpz_t a, mpz_t b) {
    mpz_t temp, temp_a, temp_b;
    mpz_init(temp);
    mpz_init_set(temp_a, a);
    mpz_init_set(temp_b, b);
    while (mpz_cmp_ui(temp_b, 0) != 0) {
        mpz_set(temp, temp_b);
        mpz_mod(temp_b, temp_a, temp_b);
        mpz_set(temp_a, temp);
    }
    mpz_set(d, temp_a);
    mpz_clears(temp, temp_a, temp_b, NULL);
}

// Function to calculate the inverse of a modulus.
// Takes two mpz_t's, a and n, calculates a mod n,
// and passes the inverse of the result of that operation
// out through mpz_t i.
void mod_inverse(mpz_t i, mpz_t a, mpz_t n) {
    // Variables for first two operations in while
    mpz_t e, r, q, temp_r, temp_e, temp_qe;
    mpz_inits(q, temp_r, temp_e, temp_qe, NULL);
    mpz_init_set(r, n);
    mpz_init_set(e, a);

    // Variables for last operation in while
    mpz_t t, y, temp_t, temp_y, temp_qy;
    mpz_inits(temp_t, temp_y, temp_qy, NULL);
    mpz_init_set_ui(t, 0);
    mpz_init_set_ui(y, 1);

    while (mpz_cmp_ui(e, 0) != 0) {
        // Set q to (n/a)
        mpz_fdiv_q(q, r, e);

        // Auxiliary variables.
        mpz_set(temp_r, r);
        mpz_set(temp_e, e);

        // Set (n, a) to (a, n-q*a)
        mpz_set(r, e);
        mpz_mul(temp_qe, q, temp_e);
        mpz_sub(e, temp_r, temp_qe);

        // Auxiliary variables.
        mpz_set(temp_t, t);
        mpz_set(temp_y, y);

        // Set (t, y) to (y, t-q*y)
        mpz_set(t, y);
        mpz_mul(temp_qy, q, temp_y);
        mpz_sub(y, temp_t, temp_qy);
    }
    if (mpz_cmp_ui(r, 1) > 0) {
        mpz_set_ui(i, 0);
        mpz_clears(e, r, q, temp_r, temp_e, temp_qe, t, y, temp_t, temp_y, temp_qy, NULL);
        return;
    }
    if (mpz_cmp_ui(t, 0) < 0) {
        mpz_add(t, t, n);
    }
    mpz_set(i, t);
    mpz_clears(e, r, q, temp_r, temp_e, temp_qe, t, y, temp_t, temp_y, temp_qy, NULL);
}

// Function to calculate the power modulus of mpz_t base,
// using mpz_t exponent as the exponent and mpz_t modulus
// as the modulus, and passing the calculated value out through
// mpz_t out.
void pow_mod(mpz_t out, mpz_t base, mpz_t exponent, mpz_t modulus) {
    mpz_t v, p, temp_exp, temp_mod, temp_vp, temp_pp;
    mpz_inits(temp_vp, temp_pp, NULL);
    mpz_init_set_ui(v, 1);
    mpz_init_set(p, base);
    mpz_init_set(temp_exp, exponent);
    mpz_init_set(temp_mod, modulus);

    while (mpz_cmp_ui(temp_exp, 0) > 0) {
        if (mpz_odd_p(temp_exp) != 0) {
            mpz_mul(temp_vp, v, p);
            mpz_mod(v, temp_vp, temp_mod);
        }
        mpz_mul(temp_pp, p, p);
        mpz_mod(p, temp_pp, temp_mod);
        mpz_fdiv_q_ui(temp_exp, temp_exp, 2);
    }
    mpz_set(out, v);
    mpz_clears(v, p, temp_exp, temp_mod, temp_vp, temp_pp, NULL);
}

// Function to test if mpz_n is prime, using iters
// number of iterations in the Miller-Rabin primality test.
//
// Returns true if n is prime, false if it it not.
//
// This function has an astronomically low chance of producing a false negative
// as long as iters is 50 or higher, but this is both incredibly unlikely and
// not harmful to the program (it just makes it test a different number).
bool is_prime(mpz_t n, uint64_t iters) {

    // Writes n - 1 = (2^s)r such that r is odd
    mpz_t temp_n, s, r, a, y, j, mod, two, temp_n_minus_one, s_minus_one;
    mpz_inits(r, a, y, j, mod, NULL);
    mpz_init_set(temp_n, n);
    mpz_init_set(temp_n_minus_one, temp_n);
    mpz_sub_ui(temp_n_minus_one, temp_n_minus_one, 1);

    mpz_init_set_ui(s, 0);
    mpz_init_set_ui(two, 2);

    if (mpz_cmp_ui(temp_n, 2) < 0) {
        mpz_clears(temp_n, s, r, a, y, j, mod, two, temp_n_minus_one, NULL);
        return false;
    }
    if (mpz_cmp_ui(temp_n, 2) == 0) {
        mpz_clears(temp_n, s, r, a, y, j, mod, two, temp_n_minus_one, NULL);
        return true;
    }
    mpz_mod_ui(mod, temp_n, 2);
    if ((mpz_cmp_ui(temp_n, 2) != 0) && (mpz_cmp_ui(mod, 0) == 0)) {
        mpz_clears(temp_n, s, r, a, y, j, mod, two, temp_n_minus_one, NULL);
        return false;
    }
    mpz_set(r, temp_n);
    mpz_sub_ui(r, r, 1);
    mpz_mod_ui(mod, r, 2);
    while (mpz_cmp_ui(mod, 0) == 0) {
        mpz_add_ui(s, s, 1);
        mpz_fdiv_q_ui(r, r, 2);
        mpz_mod_ui(mod, r, 2);
    }

    mpz_init_set(s_minus_one, s);
    mpz_sub_ui(s_minus_one, s_minus_one, 1);

    // Actual primality checking
    for (uint64_t i = 1; i <= iters; i++) {
        mpz_sub_ui(temp_n, temp_n, 4);
        mpz_urandomm(a, state, temp_n);
        mpz_add_ui(temp_n, temp_n, 4);

        mpz_add_ui(a, a, 2);
        pow_mod(y, a, r, temp_n);

        if ((mpz_cmp_ui(y, 1) != 0) && (mpz_cmp(y, temp_n_minus_one) != 0)) {

            mpz_set_ui(j, 1);

            while ((mpz_cmp(j, s_minus_one) <= 0) && (mpz_cmp(y, temp_n_minus_one) != 0)) {

                pow_mod(y, y, two, temp_n);

                if (mpz_cmp_ui(y, 1) == 0) {
                    mpz_clears(
                        temp_n, s, r, a, y, j, mod, two, temp_n_minus_one, s_minus_one, NULL);
                    return false;
                }

                mpz_add_ui(j, j, 1);
            }

            if (mpz_cmp(y, temp_n_minus_one) != 0) {
                mpz_clears(temp_n, s, r, a, y, j, mod, two, temp_n_minus_one, s_minus_one, NULL);
                return false;
            }
        }
    }

    mpz_clears(temp_n, s, r, a, y, j, mod, two, temp_n_minus_one, s_minus_one, NULL);
    return true;
}

// Function to create a prime number with uint64_t bits number of bits,
// testing the prime number with uint64_t iters number of iterations, and
// returning a valid prime number out through mpz_t p.
void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    bool success = false;
    mpz_t out;
    mpz_init(out);

    while (success == false) {
        bits += 1;
        mpz_rrandomb(out, state, bits);
        bits -= 1;

        success = is_prime(out, iters);
    }
    mpz_set(p, out);
    mpz_clear(out);
}
