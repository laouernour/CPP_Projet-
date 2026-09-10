QT += core gui widgets testlib
CONFIG += c++17 console testcase release
CONFIG -= app_bundle
TEMPLATE = app
TARGET = bank_scene_test
INCLUDEPATH += $$PWD/..
HEADERS += $$PWD/../BankScene.h $$PWD/../MainWindow.h $$PWD/../StatisticsView.h
SOURCES += $$PWD/BankSceneTest.cpp $$PWD/../BankScene.cpp $$PWD/../MainWindow.cpp $$PWD/../StatisticsView.cpp
include(../bank_core.pri)
