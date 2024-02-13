#ifndef DUCOID_H
#define DUCOID_H

#include <string>
#include <stdint.h>
#include "device.h"
#include "rand.h"

class DucoID {
    public:
        std::string generateDucoID(std::string device);
    private:
        std::string generateArduinoID();
        std::string generateESP8266ID();
        std::string generateESP32ID();
};

#endif