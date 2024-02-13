#include "rand.h"

rng_t rng;

/**
 * Initialize the random number generator
*/
void Rand::init() {
    rng_t rng;
    rng.seed(std::time(0));
}

/**
 * Get a random number
*/
int Rand::getRandom() {
    return rng();
}