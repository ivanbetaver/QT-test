cmake_minimum_required(VERSION 3.16)
project(Client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

qt6_standard_project_setup()

add_executable(Client
    main.cpp
    TcpClient.h
    TcpClient.cpp
)

target_link_libraries(Client Qt6::Core Qt6::Network)
