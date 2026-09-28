QT += widgets sql
CONFIG += console c++17
CONFIG -= app_bundle
TARGET = verse_fit
macx: QMAKE_CXXFLAGS += -include arm_acle.h
SOURCES += verse_fit.cpp \
    ../src/sources/imagegenerator.cpp \
    ../src/sources/settings.cpp \
    ../src/sources/displaysetting.cpp \
    ../src/sources/spfunctions.cpp
HEADERS += ../src/headers/imagegenerator.hpp
