// backend/src/main.cpp
#include "SerialWin32.hpp"
#include "SseServerWin32.hpp"
#include "PacketParser.hpp"
#include "CsvLogger.hpp"
#include "Util.hpp"
#include <iostream>

int main(int argc, char** argv) {
#ifndef _WIN32
    std::cerr << "This starter target is Windows-first; Linux support can be added with termios.\n";
#endif
    std::vector<std::string> args(argv+1, argv+argc);
    std::string port = arg_val(args, std::string("--serial"), "COM5");
    std::string baud_s = arg_val(args, std::string("--baud"), "57600");
    std::string team = arg_val(args, std::string("--team"), "2024-ASI-XXX");
    bool send_start = arg_has(args, std::string("--send-start"));
    uint16_t sse_port = static_cast<uint16_t>(std::stoi(arg_val(args, "--sse-port", "9090")));

    DWORD baud = std::stoul(baud_s);

    SerialWin32 serial;
    if (!serial.open(port, baud)) {
        std::cerr << "Failed to open serial " << port << " at " << baud << "\n";
        return 1;
    }
    if (send_start) {
        serial.write_bytes("START\n"); // optional per GS command requirement
    }

    SseServerWin32 sse;
    if (!sse.start(sse_port)) {
        std::cerr << "Failed to start SSE server on port " << sse_port << "\n";
        return 1;
    }

    TelemetryPacket ref;
    ref.team_id = team;
    CsvLogger logger;
    std::string csvname = "Flight_" + team + ".csv";
    if (!logger.open_with_header(csvname, ref)) {
        std::cerr << "Failed to open CSV for logging: " << csvname << "\n";
        return 1;
    }

    std::cout << "GCS backend running. Serial=" << port << " baud=" << baud << " team=" << team
              << " SSE=http://127.0.0.1:" << sse_port << "/events\n";

    for (;;) {
        auto line = serial.read_line();
        if (!line.has_value()) { Sleep(10); continue; }
        auto pkt = PacketParser::parse_line(line.value());
        if (!pkt.has_value()) continue;

        // enforce team id if incoming includes it
        if (pkt->team_id.empty()) pkt->team_id = team;

        logger.append(*pkt);
        sse.broadcast_json(pkt->to_json());
    }
    return 0;
}
