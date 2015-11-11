CONFIG += c++14
CONFIG += console
QT     += core
QT     += gui
QT     += widgets
QT     += network

TARGET = FengYunReader
TEMPLATE = app
INCLUDEPATH += $$PWD

SOURCES += $$PWD/main.cpp
SOURCES += $$PWD/MainWindow.cpp

HEADERS += $$PWD/MainWindow.hpp
HEADERS += $$PWD/MainWindowPrivate.hpp

FORMS   +=

include( $$PWD/ItemView/ItemView.pri )
include( $$PWD/ChapterListView/ChapterListView.pri )
include( $$PWD/TextView/TextView.pri )
include( $$PWD/FengYunCore/FengYunCore.pri )

CONFIG(debug,debug|release){
DEFINES *= _DEBUG
}else{
DEFINES *= QT_NO_DEBUG
DEFINES *= NDEBUG
}

#######
#     #
#     #
#     #
#     #
#######






