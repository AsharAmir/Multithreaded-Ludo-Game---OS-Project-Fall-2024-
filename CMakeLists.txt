cmake_minimum_required(VERSION 3.10)
project(LudoGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(SOURCES
    main.cpp
    ludo_game.cpp
    player.cpp
    token.cpp
    dice.cpp
    constants.cpp
    main_menu.cpp
)

add_executable(LudoGame ${SOURCES})
target_link_libraries(LudoGame Qt5::Widgets)
