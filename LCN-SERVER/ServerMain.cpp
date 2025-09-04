#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#pragma comment(lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "CommonHeader.hpp"
#include "WS2VersionInfo.hpp"
#include "BaseLog.hpp"

WS2VersionInfo ws2VersionInfo;
WS2Log ws2Logger;
NSLog nativeServerLog;

const uint16_t requiredVersion = ws2VersionInfo.GetVersion();
const uint16_t communicationPort = ws2VersionInfo.GetCommunicationPort();
const std::string serverAddress = ws2VersionInfo.GetAddress();

void startCommunication();
std::tuple<std::string, std::string> deserializeData(const std::string& serializedData);

int main(int argc, char* argv[])
{
    SetConsoleTitleA("Server");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(requiredVersion, 0x00), &wsaData) == 0)
    {
        if (LOBYTE(wsaData.wVersion) >= requiredVersion)
        {
            ws2Logger.Log("Version check verified. No errors to report.");
            startCommunication();
        }
        else {
            ws2Logger.Log("Requested version is not supported.");
            return EXIT_FAILURE;
        }
    }
    else {
        ws2Logger.Log("WinSock2 initial startup failed. Exiting program...");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

std::tuple<std::string, std::string> deserializeData(const std::string& serializedData)
{
    std::istringstream iStringStream(serializedData);
    std::string firstIndex, secondIndex;

    std::getline(iStringStream, firstIndex, '|');
    std::getline(iStringStream, secondIndex, '|');

    return std::make_tuple(firstIndex, secondIndex);
}

void startCommunication()
{
    SOCKET fdCommunication = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fdCommunication == INVALID_SOCKET)
    {
        nativeServerLog.Log("Failed to initialize communications socket for the local server.");
        WSACleanup();
        return;
    }
    else {
        sockaddr_in saCommunication;
        saCommunication.sin_family = AF_INET;
        saCommunication.sin_port = htons(communicationPort);
        saCommunication.sin_addr.s_addr = inet_addr(serverAddress.c_str());

        socklen_t bindResult = bind(fdCommunication, reinterpret_cast<sockaddr*>(&saCommunication), sizeof(saCommunication));
        if (bindResult == SOCKET_ERROR)
        {
            nativeServerLog.Log("Failed to bind to local server socket.");
            WSACleanup();
            return;
        }
        socklen_t listenResult = listen(fdCommunication, SOMAXCONN);
        if (listenResult == SOCKET_ERROR)
        {
            nativeServerLog.Log("Failed to initialize listener for local server socket.");
            WSACleanup();
            return;
        }

        nativeServerLog.Log("Starting communication service...");

        sockaddr_in saClient;
        socklen_t saClientSize = sizeof(saClient);

        SOCKET clientSocket = accept(fdCommunication, reinterpret_cast<sockaddr*>(&saClient), &saClientSize);
        if (clientSocket == SOCKET_ERROR)
        {
            nativeServerLog.Log("Failed to accept incoming connection on local server socket.");
            WSACleanup();
            return;
        }

        std::array<char, NI_MAXHOST> clientHostname;
        std::array<char, NI_MAXSERV> clientService;

        if (getnameinfo(reinterpret_cast<sockaddr*>(&saClient), saClientSize, clientHostname.data(), static_cast<DWORD>(clientHostname.size()), clientService.data(), static_cast<DWORD>(clientService.size()), 0) == 0)
        {
            nativeServerLog.CreateNSLogStream() << clientHostname.data() << " connected on port " << clientService;
        }
        else {
            inet_ntop(AF_INET, &saClient.sin_addr, clientHostname.data(), NI_MAXHOST);
            nativeServerLog.CreateNSLogStream() << clientHostname.data() << " connected on port " << ntohs(saClient.sin_port);
        }

        closesocket(fdCommunication);

        std::vector<char> messageBuffer(4096);

        while (true)
        {
            std::fill(messageBuffer.begin(), messageBuffer.end(), 0);

            int recvBytes = recv(clientSocket, messageBuffer.data(), static_cast<int>(messageBuffer.size()), 0);

            if (recvBytes <= 0)
            {
                break;
            }

            if (recvBytes > 0)
            {
                std::string recvMessage(messageBuffer.data(), recvBytes);

                while (!recvMessage.empty() && (recvMessage.back() == '\n' || recvMessage.back() == '\r'))
                {
                    recvMessage.pop_back();
                }

                const std::tuple<std::string, std::string>& clientData = deserializeData(recvMessage);
                std::string clientIdentity = std::get<0>(clientData);
                std::string clientMessage = std::get<1>(clientData);
                nativeServerLog.ClientMessage(clientIdentity, clientMessage);
            }


            if (send(clientSocket, messageBuffer.data(), recvBytes, 0) == SOCKET_ERROR)
            {
                nativeServerLog.CreateNSLogStream() << clientHostname.data() << " has disconnected." << '\n';
                WSACleanup();
                break;
            }
        }

        closesocket(clientSocket);
    }
}