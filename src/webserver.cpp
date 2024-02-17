#include "webserver.h"

const char *httpHeader = "HTTP/1.1 200 OK\n"
                        "Content-Type: text/html\n"
                        "Connection: close\n"
                        "Refresh: 10\n"
                        "\n";

const char *walletDiv = "<div class=\"wallet\">"
                        "<table class=\"walletTable\">"
                        "<tr><th>Wallet</th><th>%s</th></tr>"
                        "<tr><th>Hashrate</th><th>%d h/s</th></tr>"
                        "<tr><th>Rig Identifier</th><th>%s</th></tr>"
                        "<tr><th>Workers</th><th>%d</th></tr>"
                        "<tr><th>Device</th><th>%s</th></tr>"
                        "<tr><th>Accepted Shares</th><th>%d</th></tr>"
                        "<tr><th>Rejected Shares</th><th>%d</th></tr>"
                        "<tr><th>Ignored Shares</th><th>%d</th></tr>"
                        "</table>"
                        "</div>";

const char *style = "<style>"
                    "body {"
                    "    font-family: Arial, sans-serif;"
                    "    background-color: #141414;"
                    "    color: white;"
                    "    margin: 0;"
                    "    padding: 20px;"
                    "}"
                    "h1 {"
                    "    text-align: center;"
                    "}"
                    ".walletsContainer {"
                    "    display: flex;"
                    "    justify-content: center;"
                    "    flex-wrap: wrap;"
                    "}"
                    ".wallet {"
                    "    padding: 20px;"
                    "    background-color: #202020;"
                    "    border-radius: 10px;"
                    "    box-shadow: 0 0 10px rgba(32, 32, 32, 0.1);"
                    "    margin: 10px;"
                    "}"
                    ".walletTable {"
                    "    width: 100%;"
                    "    border-collapse: collapse;"
                    "}"
                    ".walletTable th {"
                    "    text-align: left;"
                    "    padding: 10px;"
                    "}"
                    ".walletTable td {"
                    "    padding: 10px;"
                    "}"
                    "footer {"
                    "   text-align: center;"
                    "   bottom: 0;"
                    "   left: 0;"
                    "   width: 100%;"
                    "   position: fixed;"
                    "   padding-bottom: 10px"
                    "}"
                    "</style>";

const char *http_404 = "HTTP/1.1 404 Not Found\n"
                        "Content-Type: text/html\n"
                        "Connection: close\n"
                        "\n"
                        "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>";

WebServer::WebServer() {
}

/**
 * Start the server on a given port
 * @param port The port to start the server on
 * @param maxRetries The maximum number of retries to start the server (0 means infinite retries)
 * @return The result of the operation (results are defined in result.h)
*/
int WebServer::startServer(int port, int maxRetries) {
    int retries = 0;
    while (listener.listen(port) != sf::Socket::Done) {
        DEBUG_PRINT("Failed to listen on port %d\n", port);
        if (maxRetries != 0) {
            retries++;
            if (retries >= maxRetries) {
                DEBUG_PRINT("Could not listen on port %d after %d retries\n", port, maxRetries);
                return LISTEN_FAILED;
            }
        }
        listener.close();
        DEBUG_PRINT("Retrying in %d seconds\n", RETRY_SECONDS);
        std::this_thread::sleep_for(std::chrono::seconds(RETRY_SECONDS));
    }
    return SUCCESS;
}

/**
 * Blocks until a request is recieved and handles it
 * @return The result of the operation (results are defined in result.h)
*/
int WebServer::handleNextRequest() {
    sf::TcpSocket client;

    if (listener.accept(client) != sf::Socket::Done) {
        DEBUG_PRINT("Failed to accept client\n");
        return ACCEPT_FAILED;
    }
    DEBUG_PRINT("New client: %s\n", client.getRemoteAddress().toString().c_str());
    char buffer[1024] = {0};
    bool doneRecieving = false;

    // Read everything from the client
    while (!doneRecieving) {
        std::size_t recieved;
        sf::Socket::Status status = client.receive(buffer, 1024, recieved);
        if (status != sf::Socket::Done) {
            if (status == sf::Socket::Disconnected) {
                DEBUG_PRINT("Client disconnected\n");
                return SOCKET_DISCONNECTED;
            } else {
                DEBUG_PRINT("Failed to recieve message\n");
                return SOCKET_READ_ERROR;
            }
        }
        if (recieved < 1024) {
            doneRecieving = true;
        }
    }

    // check if request wants text/html
    if (std::string(buffer).find("text/html") == std::string::npos) {
        if (client.send(http_404, strlen(http_404)) != sf::Socket::Done) {
            DEBUG_PRINT("Failed to send 404\n");
            return SOCKET_WRITE_ERROR;
        }
        DEBUG_PRINT("Sent a 404. Client disconnected\n");
        client.disconnect();
        return SUCCESS;
    }

    // Prepare the response
    std::string httpResponse = httpHeader;
    httpResponse += "<html><head><title>DucoForge by tesa-klebeband</title><meta charset=\"UTF-8\"></head>"
                    "<body><h1>DucoForge</h1><div class=\"walletsContainer\">";

    for (int i = 0; i < walletCount; i++) {
        char walletHtml[1024];
        sprintf(walletHtml, walletDiv, walletNames[i].c_str(), hashrates[i], rigIdentifiers[i].c_str(), numThreads[i], devices[i].c_str(), acceptedShares[i], rejectedShares[i], ignoredShares[i]);
        httpResponse += walletHtml;
    }

    httpResponse += "</div>" + std::string(style) + "</body><footer>DucoForge © 2024 by tesa-klebeband</footer></html>";

    // Send the response
    if (client.send(httpResponse.c_str(), httpResponse.length()) != sf::Socket::Done) {
        DEBUG_PRINT("Failed to send response\n");
        return SOCKET_WRITE_ERROR;
    }

    DEBUG_PRINT("Response sent. Client disconnected\n");
    client.disconnect();
    return SUCCESS;
}

/**
 * Set the number of wallets
 * @param count The number of wallets
*/
void WebServer::setWalletCount(int count) {
    walletCount = count;
    walletNames = new std::string[count];
    hashrates = new int[count];
    acceptedShares = new int[count];
    rejectedShares = new int[count];
    ignoredShares = new int[count];
    rigIdentifiers = new std::string[count];
    numThreads = new int[count];
    devices = new std::string[count];
}

/**
 * Set the name of a wallet
 * @param index The index of the wallet
 * @param name The name of the wallet
*/
void WebServer::setWalletName(int index, std::string name) {
    walletNames[index] = name;
}

/**
 * Set the hashrate of a wallet
 * @param index The index of the wallet
 * @param hashrate The hashrate of the wallet
*/
void WebServer::setHashrate(int index, int hashrate) {
    hashrates[index] = hashrate;
}

/**
 * Set the number of accepted shares of a wallet
 * @param index The index of the wallet
 * @param acceptedShares The number of accepted shares of the wallet
*/
void WebServer::setAcceptedShares(int index, int acceptedShares) {
    this->acceptedShares[index] = acceptedShares;
}

/**
 * Set the number of rejected shares of a wallet
 * @param index The index of the wallet
 * @param rejectedShares The number of rejected shares of the wallet
*/
void WebServer::setRejectedShares(int index, int rejectedShares) {
    this->rejectedShares[index] = rejectedShares;
}

/**
 * Set the number of ignored shares of a wallet
 * @param index The index of the wallet
 * @param ignoredShares The number of ignored shares of the wallet
*/
void WebServer::setIgnoredShares(int index, int ignoredShares) {
    this->ignoredShares[index] = ignoredShares;
}

/**
 * Set the rig identifier of a wallet
 * @param index The index of the wallet
 * @param identifier The rig identifier of the wallet
*/
void WebServer::setRigIdentifier(int index, std::string identifier) {
    rigIdentifiers[index] = identifier;
}

/**
 * Set the number of threads of a wallet
 * @param index The index of the wallet
 * @param numThreads The number of threads of the wallet
*/
void WebServer::setNumThreads(int index, int numThreads) {
    this->numThreads[index] = numThreads;
}

/**
 * Set the device of a wallet
 * @param index The index of the wallet
 * @param device The device of the wallet
*/
void WebServer::setDevice(int index, std::string device) {
    devices[index] = device;
}

/**
 * Get the port the server is running on
 * @return The port the server is running on
*/
int WebServer::getPort() {
    return listener.getLocalPort();
}