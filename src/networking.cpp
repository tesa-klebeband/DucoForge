#include "networking.h"

/**
 * This function is a callback for cURL to write the response to a string
*/
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

Networking::Networking() {
}

Networking::Pool::Pool() {
}

Networking::Mining::Mining() {
}

/**
 * Request a pool from the server
 * @return The result of the operation (results are defined in result.h)
*/
int Networking::Pool::requestPool(PoolConfig *poolConfig, int numThreads) {
   // Get the pool based on free slots
    for (int i = 0; i < numPools; i++) {
        if (threadsPerPool[i] + numThreads <= MAX_THREADS_PER_POOL) {
            poolConfig->ip = poolIPs[i];
            poolConfig->name = poolNames[i];
            poolConfig->region = poolRegions[i];
            poolConfig->port = poolPorts[i];
            threadsPerPool[i] += numThreads;
            return SUCCESS;
        }
    }

    return THREAD_LIMIT_REACHED;
}

/**
 * Check the wallet on the server, that is set in the pool configuration
 * @return The result of the operation (results are defined in result.h)
*/
int Networking::Pool::checkWallet(MiningConfig *miningConfig) {
    if (lastMillis != 0) {
        int millis = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
        if (millis - lastMillis < REQUEST_WAIT) {
            std::this_thread::sleep_for(std::chrono::milliseconds(REQUEST_WAIT - (millis - lastMillis)));
        }
    }

    // Get the response from the server
    std::string response = get(("https://server.duinocoin.com/users/" + miningConfig->walletAddr).c_str());
    
    if (response.empty()) {
        DEBUG_PRINT("Failed to check wallet\n");
        return GET_ERROR;
    }

    // Parse the response and check for parsing errors
    doc = rapidjson::Document();
    doc.Parse(response.c_str());

    if (doc.HasParseError()) {
        DEBUG_PRINT("Failed to parse wallet response\n");
        DEBUG_PRINT("Error: %s\n", response.c_str());
        return PARSE_ERROR;
    }
    if (!doc.HasMember("success")) {
        DEBUG_PRINT("Wallet response is missing required fields\n");
        return FIELD_ERROR;
    }
    if (!doc["success"].IsBool()) {
        DEBUG_PRINT("Wallet response has wrong types\n");
        return TYPE_ERROR;
    }
    // Check if the wallet is valid
    if (!doc["success"].GetBool()) {
        return INVALID;
    }

    return SUCCESS;
}

// Check the mining key on the server, that is set in the pool configuration
int Networking::Pool::checkMiningKey(MiningConfig *miningConfig) {
    if (lastMillis != 0) {
        int millis = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
        if (millis - lastMillis < REQUEST_WAIT) {
            std::this_thread::sleep_for(std::chrono::milliseconds(REQUEST_WAIT - (millis - lastMillis)));
        }
    }

    // Get the response from the server
    std::string response = get(("https://server.duinocoin.com/mining_key?u=" + miningConfig->walletAddr + "&k=" + miningConfig->miningKey).c_str());
    
    if (response.empty()) {
        DEBUG_PRINT("Failed to check mining key\n");
        return GET_ERROR;
    }

    // Parse the response and check for parsing errors
    doc = rapidjson::Document();
    doc.Parse(response.c_str());

    if (doc.HasParseError()) {
        DEBUG_PRINT("Failed to parse mining key response\n");
        DEBUG_PRINT("Error: %s\n", response.c_str());
        return PARSE_ERROR;
    }
    if (!doc.HasMember("success")) {
        DEBUG_PRINT("Mining key response is missing required fields\n");
        return FIELD_ERROR;
    }
    if (!doc["success"].IsBool()) {
        DEBUG_PRINT("Mining key response has wrong types\n");
        return TYPE_ERROR;
    }
    // Check if the mining key is valid
    if (!doc["success"].GetBool()) {
        return INVALID;
    }

    return SUCCESS;
}

/**
 * Read from the socket
 * @param data The data to read into
 * @param size The size of the data
 * @param recieved The amount of data recieved
 * @return The result of the operation (results are defined in result.h) or the amount of data recieved
*/
int Networking::Mining::socketRead(void *data, std::size_t size) {
    std::size_t recieved;
    sf::Socket::Status status = socket.receive(data, size, recieved);
    if (status != sf::Socket::Done) {
        DEBUG_PRINT("Failed to read from pool: ");
        if (status == sf::Socket::Disconnected) {
            DEBUG_PRINT("Disconnected\n");
            return SOCKET_DISCONNECTED;
        } else {
            DEBUG_PRINT("Error\n");
            return SOCKET_READ_ERROR;
        }
    }
    return recieved;
}

/**
 * Write data to the socket
 * @param data The data to write
 * @param size The size of the data
 * @return The result of the operation (results are defined in result.h) or the amount of data written
*/
int Networking::Mining::socketWrite(const void *data, std::size_t size) {
    std::size_t sent;
    sf::Socket::Status status = socket.send(data, size, sent);
    if (status != sf::Socket::Done) {
        DEBUG_PRINT("Failed to write to pool: ");
        if (status == sf::Socket::Disconnected) {
            DEBUG_PRINT("Disconnected\n");
            return SOCKET_DISCONNECTED;
        } else {
            DEBUG_PRINT("Error\n");
            return SOCKET_WRITE_ERROR;
        }
    }
    return sent;
}

/**
 * Setup a connection to the mining pool
 * @return The result of the operation (results are defined in result.h)
*/
int Networking::Mining::setupMiningConnection(PoolConfig *poolConfig) {
    // Connect to the pool
    if (socket.Disconnected != sf::Socket::Disconnected) {
        socket.disconnect();
    }
    
    sf::Socket::Status status = socket.connect(poolConfig->ip, poolConfig->port, sf::seconds(SOCKET_TIMEOUT_SECONDS));
    if (status != sf::Socket::Done) {
        DEBUG_PRINT("Failed to connect to pool\n");
        return CONNECT_ERROR;
    }

    // Read the version and MOTD from the pool
    char ver[6] = {0};
    int result = socketRead(ver, 6);
    if (result < 0) {
        return result;
    }
    poolConfig->serverVersion = ver;

    char motd[1024] = {0};
    result = socketWrite("MOTD", 4);
    if (result != 4) {
        return result;
    }
    result = socketRead(motd, 1024);
    if (result < 0) {
        return result;
    }

    poolConfig->motd = motd;

    return SUCCESS;
}

/**
 * Get a job from the pool
 * @param miningJob A pointer to the mining job to store the job in
 * @return The result of the operation (results are defined in result.h), the mining job is altered
*/
int Networking::Mining::getJob(MiningJob *miningJob, MiningConfig *miningConfig) {
    std::string job = "JOB"
        + std::string(SEPERATOR) + miningConfig->walletAddr
        + std::string(SEPERATOR) + deviceDiffs[miningConfig->device]
        + std::string(SEPERATOR) + miningConfig->miningKey
        + std::string(END_TOKEN);

    // Send the job request to the pool 
    int result = socketWrite(job.c_str(), job.length());
    if (result != job.length()) {
        return result;
    }

    char jobData[128] = {0};

    // Read the job from the pool
    result = socketRead(jobData, 128);
    if (result < 0) {
        return result;
    }

    memcpy(miningJob->lastblockhash, jobData, 40);
    memcpy(miningJob->newblockhash, jobData + 41, 40);
    miningJob->difficulty = atoi(jobData + 82);

    return SUCCESS;
}

/**
 * Submit a result to the pool
 * @param result The result to submit
 * @param hashrate The hashrate of the device
 * @return The result of the operation (results are defined in result.h)
*/
int Networking::Mining::submitResult(uint32_t result, int hashrate, MiningConfig *miningConfig, PoolConfig *poolConfig) {
    std::string additional = "";
    if (deviceCores[miningConfig->device] > 1) {
        additional = std::string(SEPERATOR) + std::to_string(miningConfig->walletID);
    }
    std::string resultStr = std::to_string(result)
        + std::string(SEPERATOR) + std::to_string(hashrate)
        + std::string(SEPERATOR) + supportedDevices[miningConfig->device]
        + std::string(SEPERATOR) + miningConfig->rigIdentifier
        + std::string(SEPERATOR) + miningConfig->ducoID
        + additional
        + std::string(END_TOKEN);

    // Submit the result to the pool
    int res = socketWrite(resultStr.c_str(), resultStr.length());
    if (res != resultStr.length()) {
        return res;
    }

    char feedback[64] = {0};
    
    // Read the feedback from the pool
    res = socketRead(feedback, 64);
    if (res < 0) {
        return res;
    }

    if (memcmp(feedback, "GOOD", 4) == 0) {
        return ACCEPTED;
    } else if (memcmp(feedback, "BAD", 3) == 0) {
        DEBUG_PRINT("Rejected share: %s\n", feedback);
        return REJECTED;
    } else {
        DEBUG_PRINT("Invalid feedback: %s\n", feedback);
        return INVALID;
    }
}

/**
 * Get a response from a given URL
 * @param url The URL to get the response from
 * @return The response from the server
*/
std::string Networking::Pool::get(const char *url) {
    std::string response;
    curl = curl_easy_init();
    if (!curl) {
        DEBUG_PRINT("Failed to initialize cURL!\n");
        return response;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        DEBUG_PRINT("Failed to get %s\n", url);
    }
    return response;
}
