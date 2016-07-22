TARGET = MONSTER_fr

TEMPLATE = app

VERSION = 2.0.0

QT += \
    core \
    multimedia
QT -= \
    gui

CONFIG += \
    c++11 \
    console

DEFINES += \
    SW_VERSION=\\\"$$VERSION\\\"

CONFIG(debug, debug|release) {
    DESTDIR = $${OUT_PWD}/debug
}
CONFIG(release, debug|release) {
    DESTDIR = $${OUT_PWD}/release
}
MOC_DIR = $${DESTDIR}/.moc
OBJECTS_DIR = $${DESTDIR}/.obj
RCC_DIR = $${DESTDIR}/.rcc

INCLUDEPATH += \
    "$$PWD/include"

win32 {
    CONFIG(debug, debug|release) {
        contains(QMAKE_TARGET.arch, x86_64) {
            # Windows x64 (64bit) debug
            LIBS += \
                -L"$$PWD/lib/win/x64/debug"
        } else {
            # Windows x86 (32bit) debug
            LIBS += \
                -L"$$PWD/lib/win/x86/debug"
        }
    }
    CONFIG(release, debug|release) {
        contains(QMAKE_TARGET.arch, x86_64) {
            # Windows x64 (64bit) release
            LIBS += \
                -L"$$PWD/lib/win/x64/release"
        } else {
            # Windows x86 (32bit) release
            LIBS += \
                -L"$$PWD/lib/win/x86/release"
        }
    }
    LIBS += \
        -llibexiv2 -lxmpsdk -lzlib1 -llibexpat
}

win32 {
    RC_ICONS = 1UpMushroom256x256.ico
}

HEADERS += \
    applicationmanager.h \
    applicationutils.h \
    base.h \
    filerenamer.h \
    logmanager.h

SOURCES += \
    applicationmanager.cpp \
    applicationutils.cpp \
    base.cpp \
    filerenamer.cpp \
    logmanager.cpp \
    main.cpp

win32 {
    CONFIG(debug, debug|release) {
        OTHER_FILES += \
            $(QTDIR)/bin/Qt5Cored.dll
    }
    CONFIG(release, debug|release) {
        OTHER_FILES += \
            $(QTDIR)/bin/Qt5Core.dll
    }
}

win32 {
    for(FILE, OTHER_FILES){
        QMAKE_POST_LINK += \
            $$quote(copy /Y \"$$shell_path($${FILE})\" \"$$shell_path($${DESTDIR})\"$$escape_expand(\n\t))
    }
}
