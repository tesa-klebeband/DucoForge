#ifndef NETWORKING_H
#define NETWORKING_H

#include <iostream>
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include "debug.h"
#include "result.h"
#include "device.h"
#include "algo.h"
#include "pools.h"

#define SEPERATOR ","
#define END_TOKEN "\n"

#define REQUEST_WAIT 2000

typedef struct {
    std::string ip;
    std::string name;
    std::string region;
    std::string serverVersion;
    std::string motd;
    int port;
} PoolConfig;

typedef struct {
    std::string walletAddr;
    std::string miningKey;
    std::string device;
    std::string rigIdentifier;
    std::string ducoID;
    uint8_t walletID;
} MiningConfig;

class Networking {
    public:
        class Pool {
            public:
                Pool();
                int requestPool(PoolConfig *poolConfig,  int numMiners);
                int checkWallet(MiningConfig *miningConfig);
                int checkMiningKey(MiningConfig *miningConfig);

            private:
                CURL *curl;
                rapidjson::Document doc;
                std::string get(const char *url);
                int lastMillis = 0;
        };

        class Mining {
            public:
                Mining();
                int setupMiningConnection(PoolConfig *poolConfig);
                int getJob(MiningJob *job, MiningConfig *miningConfig);
                int submitResult(uint32_t result, int hashrate, MiningConfig *miningConfig);

            private:
                sockaddr_in poolAddr;
                int sock;
        };

    protected:
        Networking();
};

#endif