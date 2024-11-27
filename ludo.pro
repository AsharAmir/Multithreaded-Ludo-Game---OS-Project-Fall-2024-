QT += core gui widgets

TARGET = LudoGame
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    player.cpp \
    token.cpp \
    dice.cpp \
    playerthread.cpp \
    masterthread.cpp


HEADERS += \
    mainwindow.h \
    player.h \
    token.h \
    dice.h \
    playerthread.h \
    masterthread.h

CONFIG += debug