#include <iostream>
#include "ducoforge.h"
#include "result.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <config file>\n", argv[0]);
        return -1;
    }

    DuinoV3 miner;
    if(miner.loadConfig(argv[1]) != SUCCESS) return -1;
    miner.startMining();

    return 0;
}