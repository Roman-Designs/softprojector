QT += widgets sql network httpserver
CONFIG += console c++17
CONFIG -= app_bundle
TARGET = stream_test
macx: QMAKE_CXXFLAGS += -include arm_acle.h
SOURCES += stream.cpp \
    ../src/sources/streamoutput.cpp \
    ../src/sources/streamsettingswidget.cpp \
    ../src/sources/bible.cpp \
    ../src/sources/imagegenerator.cpp \
    ../src/sources/settings.cpp \
    ../src/sources/displaysetting.cpp \
    ../src/sources/spfunctions.cpp
HEADERS += ../src/headers/streamoutput.hpp
