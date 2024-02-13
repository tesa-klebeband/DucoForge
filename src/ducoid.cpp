#include "ducoid.h"

/**
 * Generate a DUCOID for a given device
 * @param device The device to generate a DUCOID for
 * @return The generated DUCOID
*/
std::string DucoID::generateDucoID(std::string device) {
    int idType = deviceIDGenerators[device];
    switch (idType) {
        case DUCOID_ARDUINO:
            return generateArduinoID();
        case DUCOID_ESP8266:
            return generateESP8266ID();
        case DUCOID_ESP32:
            return generateESP32ID();
    }
    return "";
}

/**
 * Generate a DUCOID for an Arduino device
 * @return The generated DUCOID
*/
std::string DucoID::generateArduinoID() {
    char ducoid[23];
    uint32_t uuid1 = Rand::getRandom();
    uint32_t uuid2 = Rand::getRandom();
    sprintf(ducoid, "DUCOID%08X%08X", uuid1, uuid2);
    return std::string(ducoid);
}

/**
 * Generate a DUCOID for an ESP8266 device
 * @return The generated DUCOID
*/
std::string DucoID::generateESP8266ID() {
    char ducoid[13];
    uint32_t uuid = Rand::getRandom();
    sprintf(ducoid, "DUCOID%06X", uuid >> 8);
    return std::string(ducoid);
}

/**
 * Generate a DUCOID for an ESP32 device
 * @return The generated DUCOID
*/
std::string DucoID::generateESP32ID() {
    char ducoid[19];
    uint32_t uuid1 = Rand::getRandom();
    uint16_t uuid2 = Rand::getRandom();
    sprintf(ducoid, "DUCOID%08X%04X", uuid1, uuid2);
    return std::string(ducoid);
}