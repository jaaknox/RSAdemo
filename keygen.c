#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <gmp.h>

#include "numtheory.h"
#include "randstate.h"
#include "rsa.h"

// Helper function for bit calculation.
// Takes in two mpz_t's, and sets the
// first one to the number of bits in the
// second one.
//
// Thank you to Elmer, helper for the UCSC CSE13s
// class for pseudocode for this.
void bits_num(mpz_t bit, mpz_t n) {
    mpz_t count, temp_n;
    mpz_init_set(temp_n, n);
    mpz_init_set_ui(count, 0);

    while (mpz_cmp_ui(temp_n, 0) > 0) {
        mpz_fdiv_q_ui(temp_n, temp_n, 2);
        mpz_add_ui(count, count, 1);
    }
    mpz_set(bit, count);
    mpz_clears(count, temp_n, NULL);
}

// Main function. Takes input from the command line.
// Returns 0 upon successful run.
//
// Argc is the number of arguments passed.
// Argv is a pointer array to the arguments.
int main(int argc, char **argv) {
    int opt = 0;
    uint64_t nbits = 255;
    uint64_t iters = 50;
    uint64_t seed = time(NULL);
    bool verbose = false;
    bool gotpubfile = false;
    bool gotprvfile = false;
    FILE *pbfile;
    FILE *pvfile;

    // Parse command line options.
    while ((opt = getopt(argc, argv, "hvb:i:n:d:s:")) != -1) {
        switch (opt) {
        case 'h':
            printf("SYNOPSIS\n   Generates an RSA public/private key pair.\n\nUSAGE\n   ./keygen "
                   "[-hv] [-b bits] -n pbfile -d pvfile\n\nOPTIONS\n   -h              Display "
                   "program help and usage.\n   -v              Display verbose program output.\n  "
                   " -b bits         Minimum bits needed for public key n.\n   -c confidence   "
                   "Miller-Rabin iterations for testing primes (default: 50).\n   -n pbfile       "
                   "Public key file (default: rsa.pub).\n   -d pvfile       Private key file "
                   "(default: rsa.priv).\n   -s seed         Random seed for "
                   "testing.\nknoxa@ubuntu:~/resources/asgn6$ \n");
            return 1;
        case 'v': verbose = true; break;
        case 'b': nbits = atoi(optarg); break;
        case 'i': iters = atoi(optarg); break;
        case 'n':
            pbfile = fopen(optarg, "w");
            if (pbfile == NULL) {
                perror("Failed");
                return 1;
            }
            gotpubfile = true;
            break;
        case 'd':
            pvfile = fopen(optarg, "w");
            if (pvfile == NULL) {
                perror("Failed");
                return 1;
            }
            gotprvfile = true;
            break;
        case 's': seed = atoi(optarg); break;
        }
    }

    // Open the key files if they were not opened in getopt().
    if (gotpubfile == false) {
        pbfile = fopen("rsa.pub", "w");
    }
    if (gotprvfile == false) {
        pvfile = fopen("rsa.priv", "w");
    }

    // Set PRIVATE key file permissions to 0600.
    int pvno = fileno(pvfile);
    fchmod(pvno, S_IRUSR | S_IWUSR);

    // Initialize random state using given seed.
    randstate_init(seed);

    // Make the public and private keys.
    mpz_t p, q, b, e;
    mpz_inits(p, q, b, e, NULL);

    rsa_make_pub(p, q, b, e, nbits, iters);

    mpz_t d;
    mpz_init(d);

    rsa_make_priv(d, e, p, q);

    // Get the username as an mpz_t and sign it.
    mpz_t user, sig;
    mpz_inits(user, sig, NULL);
    char *user_buf = NULL;
    if (getenv("USER") != NULL) {
        user_buf = getenv("USER");
    }
    mpz_set_str(user, user_buf, 62);

    rsa_sign(sig, user, d, b);

    // Write the keys to their files, along with verbosity
    // (to terminal) if requested.
    rsa_write_pub(b, e, sig, user_buf, pbfile);
    rsa_write_priv(b, d, pvfile);
    if (verbose == true) {
        mpz_t bit;
        mpz_init(bit);

        printf("user = %s\n", user_buf);
        bits_num(bit, sig);
        gmp_printf("s (%Zd bits) = %Zd\n", bit, sig);
        bits_num(bit, p);
        gmp_printf("p (%Zd bits) = %Zd\n", bit, p);
        bits_num(bit, q);
        gmp_printf("q (%Zd bits) = %Zd\n", bit, q);
        bits_num(bit, b);
        gmp_printf("n (%Zd bits) = %Zd\n", bit, b);
        bits_num(bit, e);
        gmp_printf("e (%Zd bits) = %Zd\n", bit, e);
        bits_num(bit, d);
        gmp_printf("d (%Zd bits) = %Zd\n", bit, d);

        mpz_clear(bit);
    }

    // Termination.
    mpz_clears(b, d, e, p, q, user, sig, NULL);
    randstate_clear();
    fclose(pbfile);
    fclose(pvfile);
}
