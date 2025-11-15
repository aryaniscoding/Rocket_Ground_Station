// backend/src/SerialWin32.hpp
#pragma once
#ifdef _WIN32

// Always include these first in any .cpp or .hpp that uses sockets
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h> // Only if you need it, and always after winsock2.h


#include <string>
#include <vector>
#include <optional>

class SerialWin32 {
    HANDLE h_ = INVALID_HANDLE_VALUE;
    OVERLAPPED ovRead_{};
public:
    bool open(const std::string& port, DWORD baud);
    std::optional<std::string> read_line();
    bool write_bytes(const std::string& s);
    void close();
    ~SerialWin32() { close(); }
};
#endif
