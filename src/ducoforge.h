#ifndef DUINOV3_H
#define DUINOV3_H

#include <chrono>
#include <thread>
#include "config.h"
#include "debug.h"
#include "networking.h"
#include "result.h"
#include "ducoid.h"
#include "algo.h"
#include "rand.h"

class DuinoV3 {
    public:
        int loadConfig(const char* path);
        void startMining();

    private:
        Config config;
        DucoID ducoID;
        DucoS1 ducoS1;
        MiningJob ***miningJobs;
        PoolConfig *poolConfigs;
        MiningConfig **miningConfigs;
        Networking::Pool poolConnection;
        Networking::Mining ***miningConnections;
        int *numAcceptedShares;
        int *numRejectedShares;

        void miningThread(int walletIndex, int threadIndex, int core);
};

#endif