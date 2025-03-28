#pragma once

#include "IncludeMe.hpp"

class WS2VI
{
private:
	const char* nAddr = ("127.0.0.1");
	const uint32_t nReqVersion = 2;
	const uint32_t nCommunicationsPort = 54000;
	const uint32_t nRegistrationPort = 54005;
	const uint32_t nStreamPort = 54012;

public:
	const char* GetAddress() const
	{
		return nAddr;
	}

	uint32_t GetCommunicationsPort() const
	{
		return nCommunicationsPort;
	}

	uint32_t GetVersion() const
	{
		return nReqVersion;
	}

	uint32_t GetRegistrationPort() const
	{
		return nRegistrationPort;
	}

	uint32_t GetStreamPort() const
	{
		return nStreamPort;
	}
};