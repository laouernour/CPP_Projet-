QT += core gui widgets
CONFIG += c++17 release
CONFIG -= app_bundle
TEMPLATE = app
TARGET = bank_sim_gui
HEADERS += BankScene.h MainWindow.h StatisticsView.h
SOURCES += main.cpp BankScene.cpp MainWindow.cpp StatisticsView.cpp
include(bank_core.pri)
