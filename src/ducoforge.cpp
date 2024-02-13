#include "ducoforge.h"

/**
 * Load the configuration file
*/
int DuinoV3::loadConfig(const char* path) {
    return config.loadConfig(path);
}

/**
 * Start the mining process
*/
void DuinoV3::startMining() {
    // Get the number of wallets and threads
    int walletCount = config.getWalletEntryCount();
    int threadCount[walletCount];

    // Allocate memory for the mining jobs, pool configs and connections
    miningJobs = new MiningJob**[walletCount];
    poolConfigs = new PoolConfig[walletCount];
    miningConnections = new Networking::Mining**[walletCount];
    miningConfigs = new MiningConfig*[walletCount];
    numAcceptedShares = new int[walletCount];
    numRejectedShares = new int[walletCount];

    for (int i = 0; i < walletCount; i++) {
        numAcceptedShares[i] = 0;
        numRejectedShares[i] = 0;

        std::string device = config.getDeviceByIndex(i);
        int cores = deviceCores[device];
        int threadCount = config.getNumberOfThreadsByIndex(i);

        miningJobs[i] = new MiningJob*[threadCount];
        miningConnections[i] = new Networking::Mining*[threadCount];
        miningConfigs[i] = new MiningConfig[threadCount];

        std::string walletAddress = config.getWalletAddressByIndex(i);
        std::string miningKey = config.getMiningKeyByIndex(i);
        std::string rigIdentifier = config.getRigIdentifierByIndex(i);

        fprintf(stderr, "Starting %d %s's for %s..   ", threadCount, device.c_str(), walletAddress.c_str());

        int walletIDs[threadCount];
        memset(walletIDs, 0, sizeof(walletIDs));

        for (int j = 0; j < threadCount; j++) {
            miningJobs[i][j] = new MiningJob[cores];
            miningConnections[i][j] = new Networking::Mining[cores];
            miningConfigs[i][j].walletAddr = walletAddress;
            miningConfigs[i][j].miningKey = miningKey;
            miningConfigs[i][j].device = device;
            miningConfigs[i][j].rigIdentifier = rigIdentifier;
            miningConfigs[i][j].walletID = j; 
            miningConfigs[i][j].ducoID = ducoID.generateDucoID(device);
        }

        if (poolConnection.checkWallet(&miningConfigs[i][0]) == SUCCESS) {
            if (poolConnection.checkMiningKey(&miningConfigs[i][0]) == SUCCESS) {
                if (poolConnection.requestPool(&poolConfigs[i], threadCount * cores) == SUCCESS) {
                    DEBUG_PRINT("Got pool %s for wallet %s\n", poolConfigs[i].name.c_str(), walletAddress.c_str());
                } else {
                    fprintf(stderr, "Failed to request pool for wallet %s\n", walletAddress.c_str());
                    exit(-1);
                }
            } else {
                fprintf(stderr, "Invalid mining key %s\n", miningKey.c_str());
                exit(-1);
            }
        } else {
            fprintf(stderr, "Invalid wallet %s\n", walletAddress.c_str());
            exit(-1);
        }

        for (int j = 0; j < threadCount; j++) {
            for (int k = 0; k < cores; k++) {
                if (miningConnections[i][j][k].setupMiningConnection(&poolConfigs[i]) != SUCCESS) {
                    fprintf(stderr, "Failed to setup mining connection for wallet %s, thread: %d, core: %d\n", walletAddress.c_str(), j, k);
                    exit(-1);
                }
                std::thread miningThread(&DuinoV3::miningThread, this, i, j, k);
                miningThread.detach();
            }
        }
        fprintf(stderr, "DONE\n");
    }

    while (true) {
        fprintf(stderr, "\x1B[2J\x1B[H");
        for (int i = 0; i < walletCount; i++) {
            fprintf(stderr, "Wallet: %s\n", config.getWalletAddressByIndex(i).c_str());
            fprintf(stderr, "      Device:          %s\n", config.getDeviceByIndex(i).c_str());
            fprintf(stderr, "      Hashrate:        %.2f kH/s\n", (float) (config.getHashrateByIndex(i) * deviceCores[config.getDeviceByIndex(i)] * config.getNumberOfThreadsByIndex(i)) / 1000.0);
            fprintf(stderr, "      Accepted shares: %d\n", numAcceptedShares[i]);
            fprintf(stderr, "      Rejected shares: %d\n\n", numRejectedShares[i]);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

/**
 * The mining thread
 * @param walletIndex The index of the wallet
 * @param threadIndex The index of the thread
 * @param core The number of the core
*/
void DuinoV3::miningThread(int walletIndex, int threadIndex, int core) {
    DEBUG_PRINT("Started core %d thread %d for wallet %s\n", core, threadIndex,miningConfigs[walletIndex][threadIndex].walletAddr.c_str());

    // Get the hashrate and number of cores for the device, so we can calculate the hashrate for each core
    int numCores = deviceCores[config.getDeviceByIndex(walletIndex)];
    int hashrate = config.getHashrateByIndex(walletIndex) / numCores;

    while (true) {
        // Get the current time in milliseconds needed for waiting before submitting the result
        unsigned long millis = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();

        // Get the job for the connection
        if (miningConnections[walletIndex][threadIndex][core].getJob(&miningJobs[walletIndex][threadIndex][core], &miningConfigs[walletIndex][threadIndex]) != SUCCESS) {
            DEBUG_PRINT("Failed to get job for wallet %s\n", miningConfigs[walletIndex][threadIndex].walletAddr.c_str());
            continue;
        }

        // Get the result of the DUCO-S1 algorithm for the job
        uint32_t result = ducoS1.getHashResult(&miningJobs[walletIndex][threadIndex][core]);
        if (result == HASH_NOT_FOUND) {
            DEBUG_PRINT("Failed to find hash for wallet %s\n", miningConfigs[walletIndex][threadIndex].walletAddr.c_str());
        }
        
        // Calculate the time we need to wait before submitting the result, so the hashrate sounds legit to the pool
        float waitTime = (1000 / (float) hashrate) * (float) result;
        int delay = waitTime - (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - millis);
        if (delay > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay));

        // Submit the result to the pool
        int state = miningConnections[walletIndex][threadIndex][core].submitResult(result, hashrate, &miningConfigs[walletIndex][threadIndex]);
        
        if (state == ACCEPTED) {
            numAcceptedShares[walletIndex]++;
        } else if (state == REJECTED) {
            numRejectedShares[walletIndex]++;
        } else {
            numRejectedShares[walletIndex]++;
        }
    }
}