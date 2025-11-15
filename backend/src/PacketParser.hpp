// backend/src/PacketParser.hpp
#pragma once
#include "Packet.hpp"
#include <string>
#include <optional>

struct PacketParser {
    static std::string trim(const std::string& s);
    static std::vector<std::string> split(const std::string& s, char delim);
    static std::vector<double> split_numbers_flexible(const std::string& s);
    static std::optional<TelemetryPacket> parse_line(const std::string& line);
};
