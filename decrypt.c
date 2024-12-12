#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <gmp.h>

#include "numtheory.h"
#include "randstate.h"
#include "rsa.h"

// Helper function for bit calculation
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
    bool gotprvfile = false;
    bool gotinfile = false;
    bool gotoutfile = false;
    FILE *pvfile;
    FILE *infile;
    FILE *outfile;

    // Parse command line options.
    while ((opt = getopt(argc, argv, "hvi:o:n:")) != -1) {
        switch (opt) {
        case 'h':
            printf("SYNOPSIS\n   Decrypts data using RSA decryption.\n   Encrypted data is "
                   "encrypted by the encrypt program.\n\nUSAGE\n   ./decrypt [-hv] [-i infile] [-o "
                   "outfile] -n pubkey -d privkey\n\nOPTIONS\n   -h              Display program "
                   "help and usage.\n   -v              Display verbose program output.\n   -i "
                   "infile       Input file of data to decrypt (default: stdin).\n   -o outfile    "
                   "  Output file for decrypted data (default: stdout).\n   -d pvfile       "
                   "Private key file (default: rsa.priv).\n");
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
            pvfile = fopen(optarg, "r");
            if (pvfile == NULL) {
                perror("Failed");
                return 1;
            }
            gotprvfile = true;
            break;
        }
    }

    // Open the key files if they were not opened in getopt().
    if (gotprvfile == false) {
        pvfile = fopen("rsa.priv", "r");
    }
    if (gotinfile == false) {
        infile = stdin;
    }
    if (gotoutfile == false) {
        outfile = stdout;
    }

    // Read the key.
    mpz_t n, d;
    mpz_inits(n, d, NULL);

    rsa_read_priv(n, d, pvfile);

    // Print stats if verbose.
    if (verbose == true) {
        mpz_t bit;
        mpz_init(bit);

        bits_num(bit, n);
        gmp_printf("n (%Zd bits) = %Zd\n", bit, n);
        bits_num(bit, d);
        gmp_printf("d (%Zd bits) = %Zd\n", bit, d);

        mpz_clear(bit);
    }

    // Decryption.
    rsa_decrypt_file(infile, outfile, n, d);

    // Termination.
    fclose(pvfile);
    if (gotinfile == true) {
        fclose(infile);
    }
    if (gotoutfile == true) {
        fclose(outfile);
    }
    mpz_clears(n, d, NULL);
}
