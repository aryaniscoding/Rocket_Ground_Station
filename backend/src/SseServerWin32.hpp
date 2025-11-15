// backend/src/SseServerWin32.hpp
#pragma once
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

class SseServerWin32 {
public:
    bool start(uint16_t port);
    void stop();
    void broadcast_json(const std::string& json);
private:
    std::thread th_;
    std::atomic<bool> running_{false};
    SOCKET listen_sock_ = INVALID_SOCKET;
    std::mutex mtx_;
    std::vector<SOCKET> clients_; // SSE clients

    void loop_accept();
    static bool send_all(SOCKET s, const char* data, size_t len);
};
