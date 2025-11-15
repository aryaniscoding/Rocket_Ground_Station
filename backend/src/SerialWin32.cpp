// backend/src/SerialWin32.cpp
#ifdef _WIN32
#include "SerialWin32.hpp"

bool SerialWin32::open(const std::string& port, DWORD baud) {
    std::string path = "\\\\.\\" + port; // e.g., COM5
    h_ = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (h_ == INVALID_HANDLE_VALUE) return false;

    DCB dcb = {0}; dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(h_, &dcb)) return false;
    dcb.BaudRate = baud;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    if (!SetCommState(h_, &dcb)) return false;

    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = 50;
    to.ReadTotalTimeoutConstant = 50;
    to.ReadTotalTimeoutMultiplier = 1;
    SetCommTimeouts(h_, &to);

    memset(&ovRead_, 0, sizeof(ovRead_));
    ovRead_.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    SetupComm(h_, 1<<15, 1<<15);
    PurgeComm(h_, PURGE_RXCLEAR | PURGE_TXCLEAR);
    return true;
}

std::optional<std::string> SerialWin32::read_line() {
    if (h_ == INVALID_HANDLE_VALUE) return std::nullopt;
    static std::string buf;
    char tmp[256];
    DWORD rd = 0;

    for (;;) {
        if (!ReadFile(h_, tmp, sizeof(tmp), &rd, &ovRead_)) {
            if (GetLastError() == ERROR_IO_PENDING) {
                WaitForSingleObject(ovRead_.hEvent, 50);
                GetOverlappedResult(h_, &ovRead_, &rd, FALSE);
                ResetEvent(ovRead_.hEvent);
            } else {
                return std::nullopt;
            }
        }
        if (rd > 0) {
            buf.append(tmp, tmp + rd);
            // find CR or LF
            auto pos = buf.find('\n');
            if (pos == std::string::npos) pos = buf.find('\r');
            if (pos != std::string::npos) {
                std::string line = buf.substr(0, pos);
                buf.erase(0, pos + 1);
                return line;
            }
        } else {
            break;
        }
    }
    return std::nullopt;
}

bool SerialWin32::write_bytes(const std::string& s) {
    if (h_ == INVALID_HANDLE_VALUE) return false;
    DWORD wr = 0;
    return WriteFile(h_, s.data(), static_cast<DWORD>(s.size()), &wr, NULL) == TRUE;
}

void SerialWin32::close() {
    if (h_ != INVALID_HANDLE_VALUE) {
        CloseHandle(h_);
        h_ = INVALID_HANDLE_VALUE;
    }
    if (ovRead_.hEvent) {
        CloseHandle(ovRead_.hEvent);
        ovRead_.hEvent = NULL;
    }
}
#endif
