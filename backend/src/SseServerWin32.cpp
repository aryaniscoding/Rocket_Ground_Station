// backend/src/SseServerWin32.cpp
#include "SseServerWin32.hpp"
#include <cstdio>

bool SseServerWin32::start(uint16_t port) {
#ifdef _WIN32
    WSADATA w; if (WSAStartup(MAKEWORD(2,2), &w) != 0) return false;

    listen_sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock_ == INVALID_SOCKET) return false;

    u_long nb = 1; ioctlsocket(listen_sock_, FIONBIO, &nb);
    int opt = 1; setsockopt(listen_sock_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_addr.s_addr = inet_addr("127.0.0.1"); addr.sin_port = htons(port);
    if (bind(listen_sock_, (sockaddr*)&addr, sizeof(addr)) < 0) return false;
    if (listen(listen_sock_, 8) < 0) return false;

    running_ = true;
    th_ = std::thread([this]() { loop_accept(); });
    return true;    
#else
    return false;
#endif
}

void SseServerWin32::loop_accept() {
#ifdef _WIN32
    fd_set rfds;
    timeval tv{0, 100000}; // 100ms
    while (running_) {
        FD_ZERO(&rfds);
        FD_SET(listen_sock_, &rfds);
        auto r = select(0, &rfds, NULL, NULL, &tv);
        if (r > 0 && FD_ISSET(listen_sock_, &rfds)) {
            SOCKET c = accept(listen_sock_, NULL, NULL);
            if (c != INVALID_SOCKET) {
                // Very simple HTTP parsing for GET /events
                char req[1024]; int n = recv(c, req, sizeof(req)-1, 0);
                if (n <= 0) { closesocket(c); continue; }
                req[n] = 0;
                std::string s(req, req + n);
                if (s.find("GET /events") == 0) {
                    const char* hdr =
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/event-stream\r\n"
                        "Cache-Control: no-cache\r\n"
                        "Connection: keep-alive\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "\r\n";
                    send_all(c, hdr, strlen(hdr));
                    // send a hello comment
                    const char* hello = ": connected\n\n";
                    send_all(c, hello, strlen(hello));
                    std::lock_guard<std::mutex> lk(mtx_);
                    u_long nb = 1; ioctlsocket(c, FIONBIO, &nb);
                    clients_.push_back(c);
                } else {
                    const char* resp = "HTTP/1.1 404 Not Found\r\nContent-Length:0\r\n\r\n";
                    send_all(c, resp, strlen(resp));
                    closesocket(c);
                }
            }
        }
        // drop closed clients
        {
            std::lock_guard<std::mutex> lk(mtx_);
            auto it = clients_.begin();
            while (it != clients_.end()) {
                char b; int r2 = recv(*it, &b, 1, 0);
                if (r2 == 0 || (r2 < 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
                    closesocket(*it);
                    it = clients_.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
#endif
}

bool SseServerWin32::send_all(SOCKET s, const char* data, size_t len) {
#ifdef _WIN32
    size_t off = 0;
    while (off < len) {
        int r = send(s, data + off, static_cast<int>(len - off), 0);
        if (r <= 0) return false;
        off += r;
    }
    return true;
#else
    return false;
#endif
}

void SseServerWin32::broadcast_json(const std::string& json) {
#ifdef _WIN32
    std::lock_guard<std::mutex> lk(mtx_);
    std::string msg = "data: " + json + "\n\n";
    for (auto it = clients_.begin(); it != clients_.end();) {
        if (!send_all(*it, msg.c_str(), msg.size())) {
            closesocket(*it);
            it = clients_.erase(it);
        } else {
            ++it;
        }
    }
#endif
}

void SseServerWin32::stop() {
#ifdef _WIN32
    running_ = false;
    if (listen_sock_ != INVALID_SOCKET) {
        closesocket(listen_sock_);
        listen_sock_ = INVALID_SOCKET;
    }
    if (th_.joinable()) th_.join();
    std::lock_guard<std::mutex> lk(mtx_);
    for (auto c : clients_) closesocket(c);
    clients_.clear();
    WSACleanup();
#endif
}
