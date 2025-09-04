#include "CommonHeader.hpp"

class WS2VersionInfo
{
private:
    const std::string serverAddress = ("127.0.0.1");
    const uint16_t requiredVersion = 2;
    const uint16_t communicationPort = 54000;

public:
    WS2VersionInfo() {}
    ~WS2VersionInfo() {}

public:
    const std::string GetAddress() const
    {
        return this->serverAddress;
    }

    const uint16_t GetVersion() const
    {
        return this->requiredVersion;
    }

    const uint16_t GetCommunicationPort() const
    {
        return this->communicationPort;
    }
};