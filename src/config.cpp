#include "config.h"

#define KEY_MISSING(wallet, key) { \
    fprintf(stderr, "Wallet %s does not contain %s\n", wallet, key); \
    return FIELD_ERROR; \
}

#define INVALID_VALUE(wallet, key) { \
    fprintf(stderr, "Wallet %s, %s has invalid value\n", wallet, key); \
    return VALUE_ERROR; \
}

#define WRONG_TYPE(wallet, key) { \
    fprintf(stderr, "Wallet %s, %s has wrong type\n", wallet, key); \
    return TYPE_ERROR; \
}

Config::Config() {
   
}

/**
 * Load a config file from a given path, parse it and validate it
 * @param path The path to the config file
 * @return The result of the operation (results are defined in result.h) 
*/
int Config::loadConfig(const char* path) {
    // Open and read the config file into a string
    std::ifstream in(path);
    if (!in.is_open()) {
        fprintf(stderr, "Failed to open config file\n");
        return FILE_ERROR;
    }
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    // Parse the config file and check for parsing errors
    config.Parse(contents.c_str());
    if (config.HasParseError()) {
        fprintf(stderr, "Failed to parse config file\n");
        return PARSE_ERROR;
    }

    // Validate the wallets key by checking its existence, type and size
    if (!config.HasMember(WALLET_OBJECT)) {
        fprintf(stderr, "Config file does not contain wallets\n");
        return FIELD_ERROR;
    }
    if (config[WALLET_OBJECT].GetType() != rapidjson::kArrayType) {
        fprintf(stderr, "Wallets key has wrong type\n");
        return TYPE_ERROR;
    }
    if (config[WALLET_OBJECT].Size() == 0) {
        fprintf(stderr, "Wallets key is empty\n");
        return FIELD_ERROR;
    }

    // Loop through each wallet entry and validate its fields
    for (int i = 0; i < config[WALLET_OBJECT].Size(); i++) {
        // Validate the wallet address key by checking its existence and type
        if (!config[WALLET_OBJECT][i].HasMember(WALLET_ADDRESS_OBJECT)) {
            KEY_MISSING(std::to_string(i).c_str(), WALLET_ADDRESS_OBJECT);
        }
        if (config[WALLET_OBJECT][i][WALLET_ADDRESS_OBJECT].GetType() != rapidjson::kStringType) {
            WRONG_TYPE(std::to_string(i).c_str(), WALLET_ADDRESS_OBJECT);
        }

        std::string walletAddress = config[WALLET_OBJECT][i][WALLET_ADDRESS_OBJECT].GetString();

        // Validate the mining key key by checking its existence and type
        if (!config[WALLET_OBJECT][i].HasMember(MINING_KEY_OBJECT)) {
            KEY_MISSING(walletAddress.c_str(), MINING_KEY_OBJECT);
        }
        if (config[WALLET_OBJECT][i][MINING_KEY_OBJECT].GetType() != rapidjson::kStringType) {
            WRONG_TYPE(walletAddress.c_str(), MINING_KEY_OBJECT);
        }

        // Validate the rig identifier key by checking its type and setting a default value if it does not exist
        if (!config[WALLET_OBJECT][i].HasMember(RIG_IDENTIFIER_OBJECT)) {
            config[WALLET_OBJECT][i].AddMember(RIG_IDENTIFIER_OBJECT, "None", config.GetAllocator());
        }
        if (config[WALLET_OBJECT][i][RIG_IDENTIFIER_OBJECT].GetType() != rapidjson::kStringType) {
            WRONG_TYPE(walletAddress.c_str(), RIG_IDENTIFIER_OBJECT);
        }

        // Validate the device key by checking its existence, type and value
        if(!config[WALLET_OBJECT][i].HasMember(DEVICE_OBJECT)) {
            KEY_MISSING(walletAddress.c_str(), DEVICE_OBJECT);
        }
        if (config[WALLET_OBJECT][i][DEVICE_OBJECT].GetType() != rapidjson::kStringType) {
            WRONG_TYPE(walletAddress.c_str(), DEVICE_OBJECT);
        }
        std::string device = config[WALLET_OBJECT][i][DEVICE_OBJECT].GetString();
        if (supportedDevices.find(device) == supportedDevices.end()) {
            INVALID_VALUE(walletAddress.c_str(), DEVICE_OBJECT);
        }

        // Validate the number of threads key by checking its type and value and setting a default value if it does not exist
        if (!config[WALLET_OBJECT][i].HasMember(NUMBER_OF_THREADS_OBJECT)) {
            config[WALLET_OBJECT][i].AddMember(NUMBER_OF_THREADS_OBJECT, 1, config.GetAllocator());
        }
        if (config[WALLET_OBJECT][i][NUMBER_OF_THREADS_OBJECT].GetType() != rapidjson::kNumberType) {
            WRONG_TYPE(walletAddress.c_str(), NUMBER_OF_THREADS_OBJECT);
        }
        if (config[WALLET_OBJECT][i][NUMBER_OF_THREADS_OBJECT].GetInt() <= 0 || 
            config[WALLET_OBJECT][i][NUMBER_OF_THREADS_OBJECT].GetInt() *  deviceCores[device] > MAX_THREADS_PER_POOL) {
            
            INVALID_VALUE(walletAddress.c_str(), NUMBER_OF_THREADS_OBJECT);
        }

        // Validate the hashrate key by checking its type and value and setting a default value if it does not exist
        if (config[WALLET_OBJECT][i].HasMember(HASHRATE_OBJECT)) {
            if (config[WALLET_OBJECT][i][HASHRATE_OBJECT].GetType() == rapidjson::kStringType) {
                if (config[WALLET_OBJECT][i][HASHRATE_OBJECT].GetString() == std::string("auto")) {
                    config[WALLET_OBJECT][i].RemoveMember(HASHRATE_OBJECT);
                    config[WALLET_OBJECT][i].AddMember(HASHRATE_OBJECT, deviceHashRates[device], config.GetAllocator());
                } else {
                    INVALID_VALUE(walletAddress.c_str(), HASHRATE_OBJECT);
                }
            } else if (config[WALLET_OBJECT][i][HASHRATE_OBJECT].GetType() != rapidjson::kNumberType) {
                WRONG_TYPE(walletAddress.c_str(), HASHRATE_OBJECT);
            } else if (config[WALLET_OBJECT][i][HASHRATE_OBJECT].GetInt() <= 0) {
                INVALID_VALUE(walletAddress.c_str(), HASHRATE_OBJECT);
            } else if (config[WALLET_OBJECT][i][HASHRATE_OBJECT].GetInt() != deviceHashRates[device]) {
                fprintf(stderr, "WARNING: Wallet entry %s, hashrate key has value different than default for device\n", walletAddress.c_str());
            }
        } else {
            config[WALLET_OBJECT][i].AddMember(HASHRATE_OBJECT, deviceHashRates[device], config.GetAllocator());
        }
    }

    return SUCCESS;
}

/**
 * Get the number of wallet entries in the config
 * @return The number of wallet entries
*/
int Config::getWalletEntryCount() {
    return config[WALLET_OBJECT].Size();
}

/**
 * Get the wallet address of a wallet entry by its index
 * @param index The index of the wallet entry
 * @return The wallet address
*/
std::string Config::getWalletAddressByIndex(int index) {
    return config[WALLET_OBJECT][index][WALLET_ADDRESS_OBJECT].GetString();
}

/**
 * Get the mining key of a wallet entry by its index
 * @param index The index of the wallet entry
 * @return The mining key
*/
std::string Config::getMiningKeyByIndex(int index) {
    return config[WALLET_OBJECT][index][MINING_KEY_OBJECT].GetString();
}

/**
 * Get the hashrate of a wallet entry by its index
 * @param index The index of the wallet entry
 * @return The hashrate
*/
int Config::getHashrateByIndex(int index) {
    return config[WALLET_OBJECT][index][HASHRATE_OBJECT].GetInt();
}

/**
 * Get the rig identifier of a wallet entry by its index
 * @param index The index of the wallet entry
 * @return The rig identifier
*/
std::string Config::getRigIdentifierByIndex(int index) {
    return config[WALLET_OBJECT][index][RIG_IDENTIFIER_OBJECT].GetString();
}

/**
 * Get the number of threads of a wallet entry by its index
 * @param index The index of the wallet entry
 * @return The number of threads
*/
int Config::getNumberOfThreadsByIndex(int index) {
    return config[WALLET_OBJECT][index][NUMBER_OF_THREADS_OBJECT].GetInt();
}

/**
 * Get the device of a wallet entry by its index
 * @param index The index of the wallet entry
 * @return The device
*/
std::string Config::getDeviceByIndex(int index) {
    return config[WALLET_OBJECT][index][DEVICE_OBJECT].GetString();
}