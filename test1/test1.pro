QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = tessie

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG(PI) {
   DEFINES += PI
#   LIBS += -lwiringPi
   LIBS += -L/usr/lib/arm-linux-gnueabihf/ -lmosquittopp
} else {
   LIBS += -L/opt/homebrew/lib -lmosquittopp
   INCLUDEPATH += /opt/homebrew/include
}

SOURCES += \
    CANmessage.cc \
    MainWindow.cpp \
    TECDisplay.cpp \
    TECExpert.cpp \
    canFrame.cc \
    driveHardware.cc \
    tLog.cc \
    ioServer.cc \
    tMosq.cc \
    tessie.cpp \
    util.cc \
    sha256.cc

HEADERS += \
    CANmessage.hh \
    MainWindow.h \
    TECData.hh \
    TECDisplay.h \
    TECExpert.h \
    TECRegister.hh \
    canFrame.hh \
    driveHardware.hh \
    tLog.hh \
    ioServer.hh \
    tMosq.hh \
    util.hh \
    sha256.hh

FORMS += \
    MainWindow.ui \
    TECDisplay.ui

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target
