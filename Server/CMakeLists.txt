cmake_minimum_required(VERSION 3.5)

project(Server VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    tcpserver.h
    tcpserver.cpp
    settingsdialog.h
    settingsdialog.cpp
    settingsdialog.ui
    clientstablemodel.h
    clientstablemodel.cpp
    clientsmessagestablemodel.h
    clientsmessagestablemodel.cpp
    clientsmanager.h
    clientsmanager.cpp
)

add_executable(Server
    ${PROJECT_SOURCES}
)

target_link_libraries(Server PRIVATE Qt6::Core Qt6::Widgets Qt6::Network)

add_custom_command(TARGET Server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../config.ini"
        "$<TARGET_FILE_DIR:Server>/config.ini"
    COMMENT "Copying config file to build directory"
)
