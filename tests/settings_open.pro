QT += widgets sql
CONFIG += console c++17
CONFIG -= app_bundle
TARGET = settings_open
SOURCES += settings_open.cpp \
    ../src/sources/biblesettingwidget.cpp \
    ../src/sources/settings.cpp \
    ../src/sources/displaysetting.cpp \
    ../src/sources/spfunctions.cpp
HEADERS += ../src/headers/biblesettingwidget.hpp
FORMS += ../src/ui/biblesettingwidget.ui
