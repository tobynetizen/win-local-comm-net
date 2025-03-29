/**
 * @file srv-main.cpp
 * @author Tobias N. H. (to***********@protonmail.com)
 * @brief Main translation unit for the server
 * @version 1.1.0.12
 * @date 2025-03-29
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <memory>
#include <stdexcept>
#include <vector>
#include <thread>
#include <mutex>
#include <mysql/jdbc.h>

#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN

constexpr int maxClients = 100;

/**
 * @brief Class API for communicating with databases via MySQL Connector/C++
 * 
 */
class MySQL
{
private:
    std::unique_ptr<sql::Connection> connection;
    const std::string hostname = "localhost";
    const std::string username = "root";
    const std::string dbName = "db";
    const unsigned int port = 3306;

public:
    /**
     * @brief Constructor initializes connection to MySQL database
     * 
     * @param password 
     */
    MySQL(const std::string& password) {
        try {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            connection.reset(driver->connect("tcp://" + hostname + ":" + std::to_string(port), username, password));
            connection->setSchema(dbName);
        } catch (const sql::SQLException& e) {
            std::cerr << "MySQL connection error: " << e.what() << std::endl;
        }
    }
    
    /**
     * @brief Executes an SQL query
     * 
     * @param query 
     * @return true 
     * @return false 
     */
    bool executeQuery(const std::string& query) {
        try {
            std::unique_ptr<sql::Statement> stmt(connection->createStatement());
            stmt->execute(query);
            return true;
        } catch (const sql::SQLException& e) {
            std::cerr << "Query execution error: " << e.what() << std::endl;
            return false;
        }
    }
};

/**
 * @brief Class API for WinSock2 related functions
 * 
 */
class ChatServer {
private:
    SOCKET serverSocket;
    std::vector<SOCKET> clients;
    std::mutex clientMutex;

public:
    /**
     * @brief Initializes server, WinSock2 setup, socket binding, port listener
     * 
     * @param port 
     */
    ChatServer(int port) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        listen(serverSocket, maxClients);
    }

    /**
     * @brief Cleans up server socket and WinSock2 resources
     * 
     */
    ~ChatServer() {
        closesocket(serverSocket);
        WSACleanup();
    }

    /**
     * @brief Accepts incoming client connections and spawns new thread for each client
     * 
     */
    void acceptClients() {
        while (true) {
            sockaddr_in clientAddr;
            int addrLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);

            if (clientSocket != INVALID_SOCKET) {
                std::lock_guard<std::mutex> lock(clientMutex);
                clients.push_back(clientSocket);
                std::thread(&ChatServer::handleClient, this, clientSocket).detach();
            }
        }
    }

    /**
     * @brief Handles communication with a connected client, receiving and broadcasting messages
     * 
     * @param clientSocket 
     */
    void handleClient(SOCKET clientSocket) {
        char buffer[1024];
        while (true) {
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) {
                closeClient(clientSocket);
                break;
            }
            buffer[bytesReceived] = '\0';
            broadcastMessage(buffer, clientSocket);
        }
    }

    /**
     * @brief Broadcasts a received message to all connected clients except the sender
     * 
     * @param message 
     * @param sender 
     */
    void broadcastMessage(const std::string& message, SOCKET sender) {
        std::lock_guard<std::mutex> lock(clientMutex);
        for (SOCKET client : clients) {
            if (client != sender) {
                send(client, message.c_str(), message.length(), 0);
            }
        }
    }

    /**
     * @brief Closes the connection for a specific client and removes them from the client list
     * 
     * @param clientSocket 
     */
    void closeClient(SOCKET clientSocket) {
        std::lock_guard<std::mutex> lock(clientMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
        closesocket(clientSocket);
    }
};

/**
 * @brief Entry point
 * 
 * @return int 
 */
int main() {
    ChatServer server(54000);
    server.acceptClients();
    return 0;
}
