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
NCLog nativeClientLog;

const uint16_t requiredVersion = ws2VersionInfo.GetVersion();
const uint16_t communicationPort = ws2VersionInfo.GetCommunicationPort();
const std::string serverAddress = ws2VersionInfo.GetAddress();
std::string clientIdentity;

void startCommunication();
std::string serializeData(const std::tuple<std::string, std::string>& tupleData);

int main(int argc, char* argv[])
{
    SetConsoleTitleA("Client");

    nativeClientLog.PromptIdentty();
    std::getline(std::cin, clientIdentity);

    WSAData wsaData;
    socklen_t wsOk = WSAStartup(MAKEWORD(requiredVersion, 0x00), &wsaData);
    if (wsOk == NULL)
    {
        if (LOBYTE(wsaData.wVersion) >= requiredVersion)
        {
            startCommunication();
        }
        else {
            ws2Logger.Log("Requested version is not supported.");
        }
    }
    else {
        ws2Logger.Log("WinSock2 initial startup failed. Exiting program...");
    }
    WSACleanup();
    return EXIT_SUCCESS;
}

std::string serializeData(const std::tuple<std::string, std::string>& tupleData)
{
    std::ostringstream oStringStream;
    oStringStream << std::get<0>(tupleData) << ',' << std::get<1>(tupleData);
    return oStringStream.str();
}

void startCommunication()
{
    SOCKET fdCommunication = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    switch (fdCommunication)
    {
    case INVALID_SOCKET:
        nativeClientLog.Log("Failed to fetch local server file descriptor.");
        WSACleanup();
        break;
    default:
        sockaddr_in saServer;
        saServer.sin_family = AF_INET;
        saServer.sin_port = htons(communicationPort);
        saServer.sin_addr.s_addr = inet_addr(serverAddress.c_str());

        socklen_t saServerSize = sizeof(saServer);

        socklen_t connectionResult = connect(fdCommunication, reinterpret_cast<sockaddr*>(&saServer), saServerSize);
        switch (connectionResult)
        {
        case SOCKET_ERROR:
            nativeClientLog.Log("Could not connect to local server on specified port.");
            WSACleanup();
            break;
        default:
            break;
        }

        std::vector<char> responseBuffer(4096);
        std::string userInput;

        do
        {
            nativeClientLog.PromptMessage();
            std::getline(std::cin, userInput);
            if (userInput.size() > 0)
            {
                std::string serializedData = serializeData(std::make_tuple(clientIdentity, userInput));
                socklen_t sendResult = send(fdCommunication, serializedData.c_str(), static_cast<int>(serializedData.size()) + 1, NULL);
                if (sendResult != SOCKET_ERROR)
                {
                    std::fill(responseBuffer.begin(), responseBuffer.end(), 0);
                    int recvBytes = recv(fdCommunication, responseBuffer.data(), 4096, NULL);
                    if (recvBytes > 0)
                    {
                        nativeClientLog.CreateNCLogStream() << "[" << clientIdentity.c_str() << "]> " << userInput.c_str();
                    }
                }
            }
        } while (userInput.size() > 0);

        closesocket(fdCommunication);
    }
}