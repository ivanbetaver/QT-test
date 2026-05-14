cmake_minimum_required(VERSION 3.16)
project(TelecomServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

qt6_standard_project_setup()

add_executable(TelecomServer
    main.cpp
    ServerMainWindow.h
    ServerMainWindow.cpp
    TcpServer.h
    TcpServer.cpp
    ClientHandler.h
    ClientHandler.cpp
    SettingsDialog.h
    SettingsDialog.cpp
)

target_link_libraries(TelecomServer Qt6::Core Qt6::Widgets Qt6::Network)
