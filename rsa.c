#include <unistd.h>
#include <stdlib.h>
#include "rsa.h"
#include "numtheory.h"

gmp_randstate_t state;

// Function to create the public rsa key.
// Accepts two mpz_t primes as input, as well as two
// uint64_t numbers: nbits, which is associated with the
// lengths of the primes, and iters, the number of Miller-Rabin iterations.
//
// Returns nothing, just makes two primes, their product, and
// the public exponent.
void rsa_make_pub(mpz_t p, mpz_t q, mpz_t n, mpz_t e, uint64_t nbits, uint64_t iters) {
    mpz_t nbits_half, pout, qout, p_minus, q_minus, denom, n_totient;
    mpz_inits(pout, qout, p_minus, q_minus, denom, n_totient, NULL);
    mpz_init_set_ui(nbits_half, nbits);
    mpz_fdiv_q_ui(nbits_half, nbits_half, 2);

    mpz_urandomm(pout, state, nbits_half);

    // Create primes p and q.
    mpz_fdiv_q_ui(nbits_half, nbits_half, 2);
    mpz_add(pout, pout, nbits_half);

    mpz_mul_ui(nbits_half, nbits_half, 4);
    mpz_sub(qout, nbits_half, pout);

    uint64_t pbits = mpz_get_ui(pout);
    uint64_t qbits = mpz_get_ui(qout);

    make_prime(p, pbits, iters);
    make_prime(q, qbits, iters);

    // Compute the totient.
    mpz_sub_ui(p_minus, p, 1);
    mpz_sub_ui(q_minus, q, 1);

    mpz_mul(n, p, q);

    mpz_mul(n_totient, p_minus, q_minus);

    // Find a suitable public exponent.
    do {
        mpz_urandomb(pout, state, nbits);
        gcd(denom, pout, n_totient);
    } while (mpz_cmp_ui(denom, 1) != 0);
    mpz_set(e, pout);

    mpz_clears(nbits_half, pout, qout, p_minus, q_minus, denom, n_totient, NULL);
}

// Function to write the necessary information to the public file.
//
// Prints the mpz_t's n, e, s, and a string of the user's name.
// Returns nothing, just write to the pbfile.
void rsa_write_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    gmp_fprintf(pbfile, "%Zx\n%Zx\n%Zx\n", n, e, s);
    fprintf(pbfile, "%s\n", username);
}

// Function to read a public key from the public key file, and use
// the information to fill mpz_t's n, e, and s, as well as char
// array username.
//
// Returns nothing, just fills variables.
void rsa_read_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    gmp_fscanf(pbfile, "%Zx\n%Zx\n%Zx\n%s\n", n, e, s, username);
}

// Function to make the necessary variables for the private key.
//
// Returns nothing, just takes in mpz_t's p and q and sets the value
// of mpz_t's d and e.
void rsa_make_priv(mpz_t d, mpz_t e, mpz_t p, mpz_t q) {
    mpz_t n, p_minus, q_minus;
    mpz_inits(n, p_minus, q_minus, NULL);
    mpz_sub_ui(p_minus, p, 1);
    mpz_sub_ui(q_minus, q, 1);
    mpz_mul(n, p_minus, q_minus);

    mod_inverse(d, e, n);

    mpz_clears(n, p_minus, q_minus, NULL);
}

// Function to write the private key information to a file.
//
// Returns nothing, just prints mpz_t's n and d to pvfile.
void rsa_write_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fprintf(pvfile, "%Zx\n%Zx\n", n, d);
}

// Function to read the private key information from a pvfile.
//
// Returns nothing, just sets the values of mpz_t's n and d.
void rsa_read_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%Zx\n%Zx\n", n, d);
}

// Function to encrypt message m using mpz_t's e and n.
//
// Returns nothing, just places result in mpz_t c.
void rsa_encrypt(mpz_t c, mpz_t m, mpz_t e, mpz_t n) {
    pow_mod(c, m, e, n);
}

// Function to encrypt file infile, using mpz_t's n and e.
//
// Returns nothing, just outputs the encrypted file to outfile.
void rsa_encrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t e) {
    mpz_t k, logn, logn_minus_one, c, m;
    mpz_inits(k, logn, logn_minus_one, c, m, NULL);

    // Calculate k.
    mpz_set_ui(logn, mpz_sizeinbase(n, 2));

    mpz_sub_ui(logn_minus_one, logn, 1);
    mpz_fdiv_q_ui(k, logn_minus_one, 8);

    // Allocate block array.
    uint64_t ki = mpz_get_ui(k);

    uint8_t *block;
    if (ki != 0) {
        block = calloc(ki, sizeof(uint8_t));
    } else {
        block = calloc(1, sizeof(uint8_t));
    }

    uint8_t workaround = 0xFF;
    block[0] = workaround;

    // Encryption.
    mpz_sub_ui(k, k, 1);
    ki -= 1;
    uint64_t nbytes = fread(&block[1], 1, ki, infile);
    while (nbytes != 0) {
        if (nbytes < ki) {
            ki = nbytes;
        }

        mpz_import(m, ki + 1, 1, 1, 1, 0, block);
        rsa_encrypt(c, m, e, n);
        gmp_fprintf(outfile, "%Zx\n", c);
        nbytes = fread(&block[1], 1, ki, infile);
    }

    free(block);
    mpz_clears(k, logn, logn_minus_one, c, m, NULL);
}

// Function to decrypt message c using mpz_t's d and n.
//
// Returns nothing, just places decrypted message into mpz_t m.
void rsa_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t n) {
    pow_mod(m, c, d, n);
}

// Function to decrypt a file infile using mpz_t's n and d.
//
// Returns nothing, just writes the decrypted file to outfile.
void rsa_decrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t d) {
    mpz_t k, logn, logn_minus_one, c, m;
    mpz_inits(k, logn, logn_minus_one, c, m, NULL);

    // Calculate k.
    mpz_set_ui(logn, mpz_sizeinbase(n, 2));

    mpz_sub_ui(logn_minus_one, logn, 1);
    mpz_fdiv_q_ui(k, logn_minus_one, 8);

    // Allocate block array.
    uint64_t ki = mpz_get_ui(k);

    uint8_t *block;
    block = calloc(ki, sizeof(uint8_t));

    // Decryption.
    int bytes_read = gmp_fscanf(infile, "%Zx\n", c);

    rsa_decrypt(m, c, d, n);

    uint64_t intj = 0;
    size_t j;

    while (bytes_read > 0) {
        mpz_export(block, &j, 1, 1, 1, 0, m);
        intj = j;

        for (uint64_t i = 1; i < intj; i++) {
            fwrite(&block[i], 1, 1, outfile);
        }
        bytes_read = gmp_fscanf(infile, "%Zx\n", c);
        rsa_decrypt(m, c, d, n);
    }

    free(block);
    mpz_clears(k, logn, logn_minus_one, c, m, NULL);
}

// Function to produce a signature s using mpz_t's m, d, and n.
//
// Returns nothing, just passes the value of the signature out through s.
void rsa_sign(mpz_t s, mpz_t m, mpz_t d, mpz_t n) {
    pow_mod(s, m, d, n);
}

// Function to verify a signature s using mpz_t's m, e, and n.
//
// Returns true if the signature is verified, false if it isn't.
bool rsa_verify(mpz_t m, mpz_t s, mpz_t e, mpz_t n) {
    mpz_t t;
    mpz_init(t);

    pow_mod(t, s, e, n);

    if (mpz_cmp(t, m) == 0) {
        mpz_clear(t);
        return true;
    } else {
        mpz_clear(t);
        return false;
    }
}
