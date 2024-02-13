#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <map>

#define DUCOID_ARDUINO 0
#define DUCOID_ESP8266 1
#define DUCOID_ESP32 2

extern std::map<std::string, std::string> supportedDevices;
extern std::map<std::string, std::string> deviceDiffs;
extern std::map<std::string, int> deviceIDGenerators;
extern std::map<std::string, int> deviceCores;
extern std::map<std::string, int> deviceHashRates;

#endif