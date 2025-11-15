# backend/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(gcs_backend LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gcs_backend
    src/main.cpp
    src/Packet.hpp
    src/PacketParser.hpp src/PacketParser.cpp
    src/CsvLogger.hpp src/CsvLogger.cpp
    src/SerialWin32.hpp src/SerialWin32.cpp
    src/SseServerWin32.hpp src/SseServerWin32.cpp
    src/Util.hpp
)

if (WIN32)
  target_link_libraries(gcs_backend ws2_32)
endif()
