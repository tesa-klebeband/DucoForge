#ifndef ALGO_H
#define ALGO_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <openssl/sha.h>
#include "result.h"

typedef struct {
    char lastblockhash[41] = {0};
    char newblockhash[41] = {0};
    int difficulty;
} MiningJob;

class DucoS1 {
    public:
        uint32_t getHashResult(MiningJob *job);
};

#endif