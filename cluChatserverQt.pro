#-------------------------------------------------
#
# Project created by QtCreator 2016-01-31T16:08:02
#
#-------------------------------------------------

QT       += core sql network
include(/home/gravit/mercurial/ASLib/ACore/ACore.pri)
include(/home/gravit/mercurial/ASLib/ANetwork/ANetwork.pri)
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtServer
TEMPLATE = app


SOURCES += main.cpp \
    mainserver.cpp

HEADERS  += \
    mainserver.h \
    config.h

FORMS    +=
