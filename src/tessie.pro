QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = tessie

CONFIG += c++17 -g

QMAKE_CXXFLAGS += -std=c++17 -g

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# -- by default, this is compiled on a Raspberry Pi
CONFIG(NOPI) {
 LIBS += -L/opt/homebrew/lib -lmosquittopp
 INCLUDEPATH += /opt/homebrew/include
} else {
 DEFINES += PI
 LIBS += -lpigpiod_if2
 LIBS += -L/usr/lib/arm-linux-gnueabihf -lmosquittopp
}

SOURCES += \
    CANmessage.cc \
    MainWindow.cc \
    canFrame.cc \
    driveHardware.cc \
    tLog.cc \
    ioServer.cc \
    tMosq.cc \
    tessie.cc \
    util.cc \
    sha256.cc

HEADERS += \
    CANmessage.hh \
    MainWindow.hh \
    canFrame.hh \
    driveHardware.hh \
    tLog.hh \
    ioServer.hh \
    tMosq.hh \
    util.hh \
    sha256.hh

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target
