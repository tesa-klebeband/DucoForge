#include "pools.h"

const std::string poolNames[] = {"europe-node-1", "europe-node-2", "europe-node-3", "europe-node-4"};
const std::string poolIPs[] = {"194.59.183.28", "194.59.183.28", "85.239.63.53", "85.239.63.53"};
const int poolPorts[] = {5881, 5729, 7538, 8376};
const std::string poolRegions[] = {"EU", "EU", "EU", "EU"};
const int numPools = 4;
int threadsPerPool[numPools] = {0, 0, 0, 0};
