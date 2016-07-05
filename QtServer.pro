#-------------------------------------------------
#
# Project created by QtCreator 2016-01-31T16:08:02
#
#-------------------------------------------------

QT       += core sql network
QT       -= gui
include(/home/gravit/code/aslib/ACore/ACore.pri)
include(/home/gravit/code/aslib/ANetwork/ANetwork.pri)
include(/home/gravit/code/aslib/ASql/ASql.pri)
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtServer
TEMPLATE = app
 QMAKE_CXXFLAGS="-std=gnu++11"
SOURCES += main.cpp \
    mainserver.cpp \
    cmd_test.cpp \
    cmd_version.cpp \
    maincommand.cpp \
    cmd_auth.cpp \
    commandlauncher.cpp

HEADERS  += \
    mainserver.h \
    config.h \
    cmd_test.h \
    cmdheader.h \
    cmd_version.h \
    maincommand.h \
    cmd_auth.h \
    commandlauncher.h

FORMS    +=
