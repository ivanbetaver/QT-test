cmake_minimum_required(VERSION 3.5)

project(Client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(PROJECT_SOURCES
    main.cpp
    tcpclient.h
    tcpclient.cpp
    eventsgenerator.h
    eventsgenerator.cpp
)

add_executable(Client
    ${PROJECT_SOURCES}
)

target_link_libraries(Client PRIVATE Qt6::Core Qt6::Network)

add_custom_command(TARGET Client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../config.ini"
        "$<TARGET_FILE_DIR:Client>/config.ini"
    COMMENT "Copying config file to build directory"
)
