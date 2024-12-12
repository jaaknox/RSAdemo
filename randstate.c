#include "randstate.h"
#include <gmp.h>

gmp_randstate_t state;

// Function to initialize the random state.
void randstate_init(uint64_t seed) {
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, seed);
}

// Function to clear the random state.
void randstate_clear(void) {
    gmp_randclear(state);
}
