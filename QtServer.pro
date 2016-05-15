#-------------------------------------------------
#
# Project created by QtCreator 2016-01-31T16:08:02
#
#-------------------------------------------------

QT       += core sql network
QT       -= gui
include(/home/gravit/code/aslib/ACore/ACore.pri)
include(/home/gravit/code/aslib/ANetwork/ANetwork.pri)
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtServer
TEMPLATE = app
 QMAKE_CXXFLAGS="-std=gnu++11"

SOURCES += main.cpp \
    mainserver.cpp

HEADERS  += \
    mainserver.h \
    config.h

FORMS    +=
