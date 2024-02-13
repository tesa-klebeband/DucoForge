#include "algo.h"

/**
 * Get the result of the DUCO-S1 algorithm for a given job
 * @param lastblockhash The last block hash
 * @param newblockhash The new block hash
 * @param difficulty The difficulty of the job
 * @return The result of the DUCO-S1 algorithm
*/
uint32_t ducos1a(char *lastblockhash, char *newblockhash, uint16_t difficulty) {
    newblockhash[40] = 0;
    lastblockhash[40] = 0;

    for (int i = 0; i < 40; i++) {
        if (newblockhash[i] >= 'a' && newblockhash[i] <= 'f') {
            newblockhash[i] -= 'a';
            newblockhash[i] += 'A';
        }
        i++;
    }

    uint8_t final_len = 40 >> 1;
    uint8_t job[104];

    for (uint8_t i = 0, j = 0; j < final_len; i += 2, j++)
        job[j] = ((((newblockhash[i] & 0x1F) + 9) % 25) << 4) + ((newblockhash[i + 1] & 0x1F) + 9) % 25;

    for (int ducos1res = 0; ducos1res < difficulty * 100 + 1; ducos1res++) {
        uint8_t hash_bytes[20];
        char str[46] = {0};
        int dsize = sprintf(str, "%s%d", lastblockhash, ducos1res);
        SHA1((unsigned char *)str, dsize, hash_bytes);
        if (memcmp(hash_bytes, job, 20) == 0) {
            return ducos1res;
        }
    }
    return HASH_NOT_FOUND;
}

/**
 * Get the result of the DUCO-S1 algorithm for a given job
 * @param job A pointer to the job to get the result for
 * @return The result of the DUCO-S1 algorithm
*/
uint32_t DucoS1::getHashResult(MiningJob *job) {
    return ducos1a(job->lastblockhash, job->newblockhash, job->difficulty);
}