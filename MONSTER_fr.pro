TARGET = MONSTER_fr

TEMPLATE = app

VERSION = 0.1.0

QT += \
    core \
    multimedia
QT -= \
    gui

CONFIG += \
    c++11 \
    console

INCLUDEPATH += \
    "$$PWD/include"

LIBS += \
    -L"$$PWD/lib" -llibexiv2 -lxmpsdk -lzlib1 -llibexpat

SOURCES += \
    main.cpp
