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
    bool verbose = false;
    bool gotpubfile = false;
    bool gotinfile = false;
    bool gotoutfile = false;
    FILE *pbfile;
    FILE *infile;
    FILE *outfile;

    // Parse command line options.
    while ((opt = getopt(argc, argv, "hvi:o:n:")) != -1) {
        switch (opt) {
        case 'h':
            printf("SYNOPSIS\n   Encrypts data using RSA encryption.\n   Encrypted data is "
                   "decrypted by the decrypt program.\n\nUSAGE\n   ./encrypt [-hv] [-i infile] [-o "
                   "outfile] -n pubkey -d privkey\n\nOPTIONS\n   -h              Display program "
                   "help and usage.\n   -v              Display verbose program output.\n   -i "
                   "infile       Input file of data to encrypt (default: stdin).\n   -o outfile    "
                   "  Output file for encrypted data (default: stdout).\n   -n pbfile       Public "
                   "key file (default: rsa.pub).\n");
            return 1;
        case 'v': verbose = true; break;
        case 'i':
            infile = fopen(optarg, "r");
            if (infile == NULL) {
                perror("Failed");
                return 1;
            }
            gotinfile = true;
            break;
        case 'o':
            outfile = fopen(optarg, "w");
            if (outfile == NULL) {
                perror("Failed");
                return 1;
            }
            gotoutfile = true;
            break;
        case 'n':
            pbfile = fopen(optarg, "r");
            if (pbfile == NULL) {
                perror("Failed");
                return 1;
            }
            gotpubfile = true;
            break;
        }
    }

    // Open the key files if they were not opened in getopt().
    if (gotpubfile == false) {
        pbfile = fopen("rsa.pub", "r");
    }
    if (gotinfile == false) {
        infile = stdin;
    }
    if (gotoutfile == false) {
        outfile = stdout;
    }

    mpz_t n, e, s, user;
    mpz_inits(n, e, s, user, NULL);
    char user_buf[256];

    rsa_read_pub(n, e, s, user_buf, pbfile);

    // Print stats if verbose.
    if (verbose == true) {
        mpz_t bit;
        mpz_init(bit);

        printf("user = %s\n", user_buf);
        bits_num(bit, s);
        gmp_printf("s (%Zd bits) = %Zd\n", bit, s);
        bits_num(bit, n);
        gmp_printf("n (%Zd bits) = %Zd\n", bit, n);
        bits_num(bit, e);
        gmp_printf("e (%Zd bits) = %Zd\n", bit, e);

        mpz_clear(bit);
    }

    mpz_set_str(user, user_buf, 62);

    // Signature verification.
    if (rsa_verify(user, s, e, n) == false) {
        printf("Signature unable to be verified.\n");
        return 1;
    }

    // Encryption.
    rsa_encrypt_file(infile, outfile, n, e);

    // Termination.
    fclose(pbfile);
    if (gotinfile == true) {
        fclose(infile);
    }
    if (gotoutfile == true) {
        fclose(outfile);
    }
    mpz_clears(n, e, s, user, NULL);
}
