CONFIG += c++14
CONFIG += console
QT     += core
QT     += gui
QT     += widgets

TARGET = FengYunReader
TEMPLATE = app


SOURCES += $$PWD/main.cpp
SOURCES += $$PWD/MainWindow.cpp

HEADERS += $$PWD/MainWindow.hpp

FORMS   +=

include( $$PWD/ItemView/ItemView.pri )

#######
#     #
#     #
#     #
#     #
#######
