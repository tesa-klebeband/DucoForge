#ifndef RAND_H
#define RAND_H

#include <random>
#include <ctime>

typedef std::mt19937 rng_t;

class Rand {
    public:
        static void init();
        static int getRandom();
};

#endif