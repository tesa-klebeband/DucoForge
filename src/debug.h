#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>

// #define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#endif