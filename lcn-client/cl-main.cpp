/**
 * @file cl-main.cpp
 * @author Tobias N. H. (to***********@protonmail.com)
 * @brief Main translation unit for the client
 * @version 0.1.0.8
 * @date 2025-03-29
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>

#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN

/**
 * @brief Class API for WinSock2 related functions
 * 
 */
class ChatClient {
private:
    SOCKET clientSocket;
    std::mutex ioMutex;

public:
    /**
     * @brief Initializes the client, WinSock2 setup, connecting to the server
     * 
     * @param serverIp 
     * @param port 
     */
    ChatClient(const std::string& serverIp, int port) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);
        serverAddr.sin_port = htons(port);

        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Failed to connect to server." << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            exit(1);
        }
    }

    /**
     * @brief Listening for messages from the server in a separate thread.
     * 
     */
    void startListening() {
        std::thread(&ChatClient::receiveMessages, this).detach();
    }

    /**
     * @brief Prints received messages from the server
     * 
     */
    void receiveMessages() {
        char buffer[1024];
        while (true) {
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::lock_guard<std::mutex> lock(ioMutex);
                std::cout << "Server: " << buffer << std::endl;
            } else {
                std::cerr << "Disconnected from server." << std::endl;
                closesocket(clientSocket);
                WSACleanup();
                exit(1);
            }
        }
    }

    /**
     * @brief Sends message to the server
     * 
     * @param message 
     */
    void sendMessage(const std::string& message) {
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    /**
     * @brief Closes connection, WinSock2 cleanup
     * 
     */
    ~ChatClient() {
        closesocket(clientSocket);
        WSACleanup();
    }
};

/**
 * Entry point of the client program.
 */
int main() {
    ChatClient client("127.0.0.1", 54000);
    client.startListening();

    std::string userInput;
    while (true) {
        std::getline(std::cin, userInput);
        client.sendMessage(userInput);
    }
    return 0;
}
