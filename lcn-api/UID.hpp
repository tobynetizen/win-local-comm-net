#pragma once

#include "IncludeMe.hpp"

class UID
{
public:
	UID() {}
	~UID() {}
protected:
	uint32_t GenerateNumber()
	{
		std::random_device rd;
		std::mt19937_64 g(rd());
		std::uniform_int_distribution<> d(0, 255);
		return d(g);
	}

	std::string GenerateHex(uint32_t stage)
	{
        if (stage == 2)
        {
            std::stringstream ss;
            for (int i = 0; i < 4; i++)
            {
                const auto rn = GenerateNumber();
                std::stringstream hex_s;
                hex_s << std::hex << rn;
                auto hex_c = hex_s.str();
                ss << (hex_c.length() < 2 ? '0' + hex_c : hex_c);
            }
            return ss.str();
        }

        if (stage == 1 || stage == 3)
        {
            std::stringstream ss;
            for (int i = 0; i < 8; i++)
            {
                const auto rn = GenerateNumber();
                std::stringstream hex_s;
                hex_s << std::hex << rn;
                auto hex_c = hex_s.str();
                ss << (hex_c.length() < 2 ? '0' + hex_c : hex_c);
            }
            return ss.str();
        }

        return "";
	}

public:
    std::string Generate()
    {
        std::array<std::string, 3> UID_l;
        UID_l[0] = static_cast<std::string>(this->GenerateHex(1));
        UID_l[1] = static_cast<std::string>(this->GenerateHex(2));
        UID_l[2] = static_cast<std::string>(this->GenerateHex(3));

        std::stringstream r;

        r << UID_l[0] << "-" << UID_l[1] << "-" << UID_l[2];
        auto rf = r.str();

        return rf;
    }
};