QT += widgets sql network httpserver qml quick printsupport multimedia multimediawidgets
CONFIG += console c++17
CONFIG -= app_bundle
TARGET = shutdown_test
macx: QMAKE_CXXFLAGS += -include arm_acle.h
APP_DIR = $$clean_path($$PWD/../src)
SOURCES = $$files($$APP_DIR/sources/*.cpp)
SOURCES -= $$APP_DIR/sources/main.cpp
# Retired pre-Qt-6 display implementation is not part of the application build.
SOURCES -= $$APP_DIR/sources/displayscreen.cpp
SOURCES += shutdown.cpp
HEADERS = $$files($$APP_DIR/headers/*.hpp)
HEADERS -= $$APP_DIR/headers/displayscreen.hpp
FORMS = $$files($$APP_DIR/ui/*.ui)
RESOURCES += $$APP_DIR/softprojector.qrc
