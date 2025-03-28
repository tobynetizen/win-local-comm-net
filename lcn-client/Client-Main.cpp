#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN 

#pragma comment(lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "IncludeMe.hpp"
#include "WS2VersionInfo.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <future>

using namespace std::literals::chrono_literals;

class CL
{
public:
	CL() {};
	~CL() {};

public:
	void log(char* user, char* resBuffer, int rBytes)
	{
		std::cout << "[" << user << "]> " << std::string(resBuffer, NULL, rBytes) << std::endl;
	}
};

WS2VI ws2vi;

#pragma region WS2Data

uint32_t nReqVersion = ws2vi.GetVersion();
uint32_t nCommunicationsPort = ws2vi.GetCommunicationsPort();
uint32_t nRegistrationPort = ws2vi.GetRegistrationPort();
const char* nAddr = ws2vi.GetAddress();

#pragma endregion

#pragma region Prototypes

void DisplayNI(std::promise<std::string> promiseObject1);
void StartRegistration(std::promise<std::string> pRegistration);
void StartCommunication(char* csDisplayName);

#pragma endregion

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	int wsOk = WSAStartup(MAKEWORD(nReqVersion, 0x00), &wsaData);
	if (wsOk == NULL)
	{
		if (LOBYTE(wsaData.wVersion >= nReqVersion))
		{
			std::promise<std::string> pRegistration;
			auto fgRegistration = pRegistration.get_future();
			std::thread mysql_reg_t(StartRegistration, std::move(pRegistration));
			std::string ffDisplayName = fgRegistration.get();
			char* csDisplayName = const_cast<char*>(ffDisplayName.c_str());

			mysql_reg_t.join();

			StartCommunication(csDisplayName);
		}
		else
		{
			std::cout << "Requested version is not supported." << std::endl;
		}
	}
	else
	{
		std::cout << "WinSock2 initial startup failed. Exiting program..." << std::endl;
	}
	WSACleanup();
	ExitProcess(EXIT_SUCCESS);
}

void DisplayNI(std::promise<std::string> pDisplayNI)
{
	std::string sDisplayName;
	std::cout << "[ENTER A DISPLAY NAME]> ";
	std::getline(std::cin, sDisplayName);
	pDisplayNI.set_value(sDisplayName);
}

void StartRegistration(std::promise<std::string> pRegistration)
{
	std::promise<std::string> pDisplayNI;
	auto fgDisplayNI = pDisplayNI.get_future();
	std::thread dpy_n_fetch_t(DisplayNI, std::move(pDisplayNI));
	std::string ffDisplayName = fgDisplayNI.get();
	const char* csDisplayName = ffDisplayName.c_str();
	pRegistration.set_value(csDisplayName);
	int dnSize = sizeof(csDisplayName);
	dpy_n_fetch_t.join();

	std::cout << "Authenticating..." << std::endl;

	SOCKET fdRegistration = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	switch (fdRegistration)
	{
	case INVALID_SOCKET:
		std::cout << "Failed to fetch local server file descriptor. Code #" << WSAGetLastError() << std::endl;
		WSACleanup();
		break;
	default:
		sockaddr_in clService;
		clService.sin_family = AF_INET;
		clService.sin_port = htons(nRegistrationPort);
		clService.sin_addr.s_addr = inet_addr(nAddr);

		int clSize = sizeof(clService);

		int connResult = connect(fdRegistration, reinterpret_cast<sockaddr*>(&clService), clSize);
		switch (connResult)
		{
		case SOCKET_ERROR:
			std::cout << "Could not connect to local server on specified port. Code #" << WSAGetLastError() << std::endl;
			WSACleanup();
			break;
		default:
			break;
		}

		int registrationResposne = send(fdRegistration, csDisplayName, dnSize + 1, NULL);
		if (registrationResposne != SOCKET_ERROR)
		{
			std::cout << "Authenticated successfully!" << std::endl;
		}
		else
		{
			std::cout << "Failed to authenticate with local server." << std::endl;
			WSACleanup();
			ExitProcess(EXIT_FAILURE);
		}
	}
}

void StartCommunication(char* csDisplayName)
{
	SOCKET fdConversation = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	switch (fdConversation)
	{
	case INVALID_SOCKET:
		std::cout << "Failed to fetch local server file descriptor. Code #" << WSAGetLastError() << std::endl;
		WSACleanup();
		break;
	default:
		sockaddr_in clService;
		clService.sin_family = AF_INET;
		clService.sin_port = htons(nCommunicationsPort);
		clService.sin_addr.s_addr = inet_addr(nAddr);

		int clSize = sizeof(clService);

		int connResult = connect(fdConversation, reinterpret_cast<sockaddr*>(&clService), clSize);
		switch (connResult)
		{
		case SOCKET_ERROR:
			std::cout << "Could not connect to local server on specified port. Code #" << WSAGetLastError() << std::endl;
			WSACleanup();
			break;
		default:
			break;
		}

		char responseBuffer[4096];
		std::string uInput;
		CL cl;

		do
		{
			std::cout << "> ";
			std::getline(std::cin, uInput);
			if (uInput.size() > 0)
			{
				int sendResult = send(fdConversation, uInput.c_str(), uInput.size() + 1, NULL);
				if (sendResult != SOCKET_ERROR)
				{
					memset(&responseBuffer, 0, sizeof(responseBuffer));
					int rBytes = recv(fdConversation, responseBuffer, 4096, NULL);
					if (rBytes > 0)
					{
						cl.log(csDisplayName, responseBuffer, rBytes);
					}
				}
			}
		} while (uInput.size() > 0);

		closesocket(fdConversation);
	}
}