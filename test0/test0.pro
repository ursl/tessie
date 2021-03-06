######################################################################
# Automatically generated by qmake (3.1) Tue Jun 30 18:22:34 2020
######################################################################

TEMPLATE = app
TARGET = tessie
INCLUDEPATH += .

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
SOURCES += main.cc \
    gui.cc \
    drivehardware.cc \
    rpcServer.cc \
    tLog.cc \
    timeChart.cc \
    timeseries.cc

HEADERS += \
    timeChart.hh \
    ui_gui.h \
    gui.hh \
    driveHardware.hh \
    rpcServer.hh \
    tLog.hh \
    timeseries.hh


QT += widgets
QT += charts

#LIBS += -L rpc -l trpc
LIBS += trpc.o

CONFIG(PI) {
   DEFINES += PI
   LIBS += -lwiringPi
} else {

}

DISTFILES +=

FORMS += \
    gui.ui
