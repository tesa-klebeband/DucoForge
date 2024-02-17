#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <iostream>
#include <SFML/Network.hpp>
#include <thread>
#include <string>
#include <chrono>
#include <cstring>
#include "debug.h"
#include "result.h"

#define HTTP_PORT 8080
#define RETRY_SECONDS 2
#define MAX_RETRIES 0

class WebServer {
    public:
        WebServer();
        int startServer(int port = HTTP_PORT, int maxRetries = MAX_RETRIES);
        int handleNextRequest();
        void setWalletCount(int count);
        void setWalletName(int index, std::string name);
        void setHashrate(int index, int hashrate);
        void setAcceptedShares(int index, int acceptedShares);
        void setRejectedShares(int index, int rejectedShares);
        void setIgnoredShares(int index, int ignoredShares);
        void setRigIdentifier(int index, std::string identifier);
        void setNumThreads(int index, int numThreads);
        void setDevice(int index, std::string device);
        int getPort();

    private:
        sf::TcpListener listener;
        int walletCount;
        std::string *walletNames;
        int *hashrates;
        int *acceptedShares;
        int *rejectedShares;
        int *ignoredShares;
        std::string *rigIdentifiers;
        int *numThreads;
        std::string *devices;
};

#endif