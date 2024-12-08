cmake_minimum_required(VERSION 3.10)
project(LudoGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(SOURCES
    i222420_i222599_i228752_main.cpp
    i222420_i222599_i228752_ludo_game.cpp
    i222420_i222599_i228752_player.cpp
    i222420_i222599_i228752_token.cpp
    i222420_i222599_i228752_dice.cpp
    i222420_i222599_i228752_constants.cpp
    i222420_i222599_i228752_main_menu.cpp
)

add_executable(LudoGame ${SOURCES})
target_link_libraries(LudoGame Qt5::Widgets)
