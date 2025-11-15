// backend/src/PacketParser.cpp
#include "PacketParser.hpp"
#include <cctype>
#include <sstream>

std::string PacketParser::trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b-1]))) --b;
    return s.substr(a, b - a);
}

std::vector<std::string> PacketParser::split(const std::string& s, char d) {
    std::vector<std::string> out; std::string cur;
    std::istringstream is(s);
    while (std::getline(is, cur, d)) out.push_back(trim(cur));
    return out;
}

std::vector<double> PacketParser::split_numbers_flexible(const std::string& s) {
    // Accept subfield delimiters: '|', '/', ';', or single number.
    std::vector<double> vals;
    std::string buf = s;
    for (char c : {'|','/',';'}) {
        for (auto& ch : buf) if (ch == c) ch = ',';
    }   
    auto parts = split(buf, ',');
    for (auto& p : parts) {
        try { vals.push_back(std::stod(p)); } catch (...) {}
    }
    return vals;
}

// std::optional<TelemetryPacket> PacketParser::parse_line(const std::string& line) {
//     auto raw = trim(line);
//     if (raw.empty()) return std::nullopt;
//     // Remove trailing CR if present
//     std::string s = raw;
//     if (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();

//     auto f = split(s, ',');
//     if (f.size() < 15) return std::nullopt; // required at least 15

//     TelemetryPacket p;
//     size_t i = 0;
//     p.team_id = f[i++];

//     try {
//         p.team_id       = f[0];
//         p.time_s        = std::stod(f[i++]);
//         p.packet_count  = static_cast<uint64_t>(std::stoll(f[i++]));
//         p.altitude_m    = std::stod(f[i++]);
//         p.pressure_pa   = std::stod(f[i++]);
//         p.temp_c        = std::stod(f[i++]);
//         p.voltage_v     = std::stod(f[i++]);
//         p.gnss_time_s   = std::stod(f[i++]);
//         p.gnss_lat_deg  = std::stod(f[i++]);
//         p.gnss_lon_deg  = std::stod(f[i++]);
//         p.gnss_alt_m    = std::stod(f[i++]);
//         p.gnss_sats     = std::stoi(f[i++]);

//         // Accelerometer / orientation bundle
//         p.accel_orient  = split_numbers_flexible(f[i++]);

//         p.gyro_spin_dps = std::stod(f[i++]);
//         p.flight_state  = f[i++];

//         if (i < f.size()) {
//             std::string rest;
//             for (; i < f.size(); ++i) {
//                 if (!rest.empty()) rest += ",";
//                 rest += f[i];
//             }
//             p.optional = rest;
//         }
//     } catch (...) {
//         return std::nullopt;
//     }
//     return p;
// }

std::optional<TelemetryPacket> PacketParser::parse_line(const std::string& line) {
    std::string raw = trim(line);
    if (raw.empty()) return std::nullopt;

    // Remove possible trailing CR or LF
    if (!raw.empty() && (raw.back() == '\r' || raw.back() == '\n'))
        raw.pop_back();

    // Split by comma
    auto f = split(raw, ',');
    if (f.size() < 15)
        return std::nullopt; // Requires at least the 15 standard fields

    TelemetryPacket p;
    size_t i = 0;

    // Keep TEAM_ID exactly as received — don't alter it
    p.team_id = f[i++];

    try {
        p.time_s        = std::stod(f[i++]);
        p.packet_count  = static_cast<uint64_t>(std::stoll(f[i++]));
        p.altitude_m    = std::stod(f[i++]);
        p.pressure_pa   = std::stod(f[i++]);
        p.temp_c        = std::stod(f[i++]);
        p.voltage_v     = std::stod(f[i++]);
        p.gnss_time_s   = std::stod(f[i++]);
        p.gnss_lat_deg  = std::stod(f[i++]);
        p.gnss_lon_deg  = std::stod(f[i++]);
        p.gnss_alt_m    = std::stod(f[i++]);
        p.gnss_sats     = std::stoi(f[i++]);
        p.accel_orient  = split_numbers_flexible(f[i++]);
        p.gyro_spin_dps = std::stod(f[i++]);
        p.flight_state  = f[i++];

        if (i < f.size()) {
            std::string opt;
            for (; i < f.size(); ++i) {
                if (!opt.empty()) opt += ",";
                opt += f[i];
            }
            p.optional = opt;
        }

    } catch (...) {
        return std::nullopt;
    }

    return p;
}
