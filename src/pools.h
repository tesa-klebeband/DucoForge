#ifndef POOL_H
#define POOL_H

#include <string>

#define MAX_THREADS_PER_POOL 50

extern const std::string poolNames[];
extern const std::string poolIPs[];
extern const int poolPorts[];
extern const std::string poolRegions[];
extern const int numPools;
extern int threadsPerPool[];

#endif