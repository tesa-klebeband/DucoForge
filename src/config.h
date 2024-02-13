#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <map>
#include "debug.h"
#include "device.h"
#include "result.h"
#include "pools.h"

#define WALLET_OBJECT "wallets"
#define WALLET_ADDRESS_OBJECT "walletAddr"
#define MINING_KEY_OBJECT "miningKey"
#define HASHRATE_OBJECT "hashrate"
#define RIG_IDENTIFIER_OBJECT "rigIdentifier"
#define NUMBER_OF_THREADS_OBJECT "numberOfThreads"
#define DEVICE_OBJECT "device"

class Config {
    public:
        Config();
        int loadConfig(const char* path);
        int getWalletEntryCount();
        std::string getWalletAddressByIndex(int index);
        std::string getMiningKeyByIndex(int index);
        int getHashrateByIndex(int index);
        std::string getRigIdentifierByIndex(int index);
        int getNumberOfThreadsByIndex(int index);
        std::string getDeviceByIndex(int index);
    
    private:
        rapidjson::Document config;
};

#endif