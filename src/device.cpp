#include "device.h"

/**
 * This file contains multiple mappings for the supported devices and their respective properties
*/
std::map<std::string, std::string> supportedDevices = {
    {"Arduino", "Official AVR Miner 4.0"},
    {"ESP8266", "Official ESP8266 Miner 4.0"},
    {"ESP32", "Official ESP32 Miner 4.0"},
    {"ESP32S", "Official ESP32-S2 Miner 4.0"}
};

std::map<std::string, std::string> deviceDiffs = {
    {"Arduino", "AVR"},
    {"ESP8266", "ESP8266H"},
    {"ESP32", "ESP32"},
    {"ESP32S", "ESP32S"}
};

std::map<std::string, int> deviceIDGenerators = {
    {"Arduino", DUCOID_ARDUINO},
    {"ESP8266", DUCOID_ESP8266},
    {"ESP32", DUCOID_ESP32},
    {"ESP32S", DUCOID_ESP32}
};

std::map<std::string, int> deviceCores = {
    {"Arduino", 1},
    {"ESP8266", 1},
    {"ESP32", 2},
    {"ESP32S", 1}
};

std::map<std::string, int> deviceHashRates = {
    {"Arduino", 343},
    {"ESP8266", 66000},
    {"ESP32", 84000},
    {"ESP32S", 82000}
};