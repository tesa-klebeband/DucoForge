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
    /* Old way of getting pool directly from server - deprecated due to maximum number of connections per pool (to limit workers)
    // Get the response from the server
    std::string response = get("https://server.duinocoin.com/getPool");
    if (response.empty()) {
        DEBUG_PRINT("Failed to get pool\n");
        return GET_ERROR;
    }

    // Parse the response and check for parsing errors
    doc = rapidjson::Document();
    doc.Parse(response.c_str());
    
    if (doc.HasParseError()) {
        DEBUG_PRINT("Failed to parse pool response\n");
        DEBUG_PRINT("Error: %s\n", response.c_str());
        return PARSE_ERROR;
    }
    if (!doc.HasMember("ip") || !doc.HasMember("name") || !doc.HasMember("region") || !doc.HasMember("port")) {
        DEBUG_PRINT("Pool response is missing required fields\n");
        return FIELD_ERROR;
    }
    if (!doc["ip"].IsString() || !doc["name"].IsString() || !doc["region"].IsString() || !doc["port"].IsInt()) {
        DEBUG_PRINT("Pool response has wrong types\n");
        return TYPE_ERROR;
    }

    // Set the pool configuration
    poolConfig->ip = doc["ip"].GetString();
    poolConfig->name = doc["name"].GetString();
    poolConfig->region = doc["region"].GetString();
    poolConfig->port = doc["port"].GetInt();
    */

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
 * Setup a connection to the mining pool
 * @return The result of the operation (results are defined in result.h)
*/
int Networking::Mining::setupMiningConnection(PoolConfig *poolConfig) {
    // Create a socket and connect to the pool
    memset(&poolAddr, 0, sizeof(poolAddr));
    poolAddr.sin_family = AF_INET;
    poolAddr.sin_port = htons(poolConfig->port);
    if (inet_pton(AF_INET, poolConfig->ip.c_str(), &poolAddr.sin_addr) <= 0) {
        DEBUG_PRINT("Failed to convert pool IP\n");
        return INVALID;
    }
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        DEBUG_PRINT("Failed to create socket\n");
        return INVALID;
    }
    if (connect(sock, (struct sockaddr *)&poolAddr, sizeof(poolAddr)) < 0) {
        DEBUG_PRINT("Failed to connect to pool\n");
        return INVALID;
    }

    // Read the version and MOTD from the pool
    char ver[6] = {0};
    if (recv(sock, ver, 6, 0) < 0) {
        DEBUG_PRINT("Failed to receive version\n");
        perror("recv");
        return GET_ERROR;
    }
    for (int i = 0; i < 6; i++) {
        if (ver[i] == '\n') {
            ver[i] = '\0';
            break;
        }
    }
    poolConfig->serverVersion = ver;

    char motd[1024] = {0};
    send(sock, "MOTD", 4, 0);
    if (recv(sock, motd, 1024, 0) < 0) {
        DEBUG_PRINT("Failed to receive MOTD\n");
        perror("recv");
        return GET_ERROR;
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

    send(sock, job.c_str(), job.length(), 0);
    char jobData[128] = {0};
    if (recv(sock, jobData, 128, 0) < 0) {
        DEBUG_PRINT("Failed to receive job\n");
        perror("recv");
        return GET_ERROR;
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
int Networking::Mining::submitResult(uint32_t result, int hashrate, MiningConfig *miningConfig) {
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
    
    send(sock, resultStr.c_str(), resultStr.length(), 0);
    char feedback[64] = {0};
    if (recv(sock, feedback, 64, 0) < 0) {
        DEBUG_PRINT("Failed to receive feedback\n");
        perror("recv");
        return GET_ERROR;
    }
    
    if (memcmp(feedback, "GOOD", 4) != 0) {
        return REJECTED;
    } else {
        return ACCEPTED;
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