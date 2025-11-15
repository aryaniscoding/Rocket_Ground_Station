// backend/src/Packet.hpp
#pragma once
#include <string>
#include <optional>
#include <vector>
#include <cstdint>

struct TelemetryPacket {
    std::string team_id;            // 1
    double time_s = 0.0;           // 2
    uint64_t packet_count = 0;     // 3
    double altitude_m = 0.0;       // 4 (relative to pad)
    double pressure_pa = 0.0;      // 5
    double temp_c = 0.0;           // 6
    double voltage_v = 0.0;        // 7
    double gnss_time_s = 0.0;      // 8
    double gnss_lat_deg = 0.0;     // 9
    double gnss_lon_deg = 0.0;     // 10
    double gnss_alt_m = 0.0;       // 11
    int gnss_sats = 0;             // 12
    // 13: accelerometer + orientation bundle; accept flexible subformat
    std::vector<double> accel_orient; 
    double gyro_spin_dps = 0.0;    // 14
    std::string flight_state;      // 15
    std::optional<std::string> optional; // 16

    std::string csv_header() const {
        return "TEAM_ID,TIME_STAMPING,PACKET_COUNT,ALTITUDE,PRESSURE,TEMP,VOLTAGE,GNSS_TIME,GNSS_LATITUDE,GNSS_LONGITUDE,GNSS_ALTITUDE,GNSS_SATS,ACCELEROMETER_DATA,GYRO_SPIN_RATE,FLIGHT_SOFTWARE_STATE,OPTIONAL_DATA";
    }

    std::string accel_joined() const {
        std::string s;
        for (size_t i = 0; i < accel_orient.size(); ++i) {
            if (i) s += '|';
            s += std::to_string(accel_orient[i]);
        }
        return s;
    }

    std::string to_csv_line() const {
        std::string opt = optional.value_or("");
        char buf[2048];
        snprintf(buf, sizeof(buf),
            "%s,%.3f,%llu,%.3f,%.1f,%.1f,%.2f,%.3f,%.4f,%.4f,%.1f,%d,%s,%.3f,%s,%s",
            team_id.c_str(), time_s, static_cast<unsigned long long>(packet_count),
            altitude_m, pressure_pa, temp_c, voltage_v, gnss_time_s, gnss_lat_deg, gnss_lon_deg,
            gnss_alt_m, gnss_sats, accel_joined().c_str(), gyro_spin_dps, flight_state.c_str(), opt.c_str());
        return std::string(buf);
    }

    std::string to_json() const {
        std::string json = "{";
        json += "\"team_id\":\"" + team_id + "\"";
        json += ",\"time_s\":" + std::to_string(time_s);
        json += ",\"packet_count\":" + std::to_string(packet_count);
        json += ",\"altitude_m\":" + std::to_string(altitude_m);
        json += ",\"pressure_pa\":" + std::to_string(pressure_pa);
        json += ",\"temp_c\":" + std::to_string(temp_c);
        json += ",\"voltage_v\":" + std::to_string(voltage_v);
        json += ",\"gnss_time_s\":" + std::to_string(gnss_time_s);
        json += ",\"gnss_lat_deg\":" + std::to_string(gnss_lat_deg);
        json += ",\"gnss_lon_deg\":" + std::to_string(gnss_lon_deg);
        json += ",\"gnss_alt_m\":" + std::to_string(gnss_alt_m);
        json += ",\"gnss_sats\":" + std::to_string(gnss_sats);
        json += ",\"accel_orient\":[";
        for (size_t i = 0; i < accel_orient.size(); ++i) {
            if (i) json += ",";
            json += std::to_string(accel_orient[i]);
        }
        json += "]";
        json += ",\"gyro_spin_dps\":" + std::to_string(gyro_spin_dps);
        json += ",\"flight_state\":\"" + flight_state + "\"";
        if (optional.has_value()) {
            json += ",\"optional\":\"" + optional.value() + "\"";
        }
        json += "}";
        return json;
    }
};
