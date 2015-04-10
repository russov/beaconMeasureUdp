#-------------------------------------------------
#
# Project created by QtCreator 2015-03-25T22:23:09
#
#-------------------------------------------------

QT       += core gui network sql

CONFIG += static c++11

QMAKE_LFLAGS += -static -static-libgcc
LIBS += -static-libgcc

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = beaconUdp
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        beacondata.cpp \
    bddata.cpp \
    parsebeacondata.cpp \
    mapcontrol.cpp \
    gpf.cpp

HEADERS  += mainwindow.h\
            beacondata.h \
    bddata.h \
    parsebeacondata.h \
    mapcontrol.h \
    gpf.h

FORMS    += mainwindow.ui \
    mapcontrol.ui

CONFIG += mobility
MOBILITY = 

