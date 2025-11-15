// backend/src/CsvLogger.hpp
#pragma once
#include "Packet.hpp"
#include <fstream>
#include <string>

class CsvLogger {
    std::ofstream file_;
public:
    bool open_with_header(const std::string& path, const TelemetryPacket& ref) {
        file_.open(path, std::ios::out | std::ios::trunc);
        if (!file_.is_open()) return false;
        file_ << ref.csv_header() << "\n";
        return true;
    }
    void append(const TelemetryPacket& p) {
        if (file_.is_open()) file_ << p.to_csv_line() << "\n";
    }
};
