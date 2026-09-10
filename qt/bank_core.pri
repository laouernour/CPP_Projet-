# Mêmes classes métier pour l'application et ses tests Qt.
BANK_ROOT = $$PWD/..
INCLUDEPATH += $$BANK_ROOT/include
HEADERS += $$BANK_ROOT/include/simulation/SimulationDefaults.h
SOURCES += \
    $$BANK_ROOT/src/bank/Bank.cpp \
    $$BANK_ROOT/src/bank/Cashier.cpp \
    $$BANK_ROOT/src/bank/Queue.cpp \
    $$BANK_ROOT/src/client/AbstractClient.cpp \
    $$BANK_ROOT/src/client/AbstractOperation.cpp \
    $$BANK_ROOT/src/client/Client.cpp \
    $$BANK_ROOT/src/client/Consultation.cpp \
    $$BANK_ROOT/src/client/Transfer.cpp \
    $$BANK_ROOT/src/client/VIPClient.cpp \
    $$BANK_ROOT/src/client/Withdraw.cpp \
    $$BANK_ROOT/src/simulation/Simulation.cpp \
    $$BANK_ROOT/src/simulation/SimulationEntry.cpp \
    $$BANK_ROOT/src/simulation/SimulationUtility.cpp \
    $$BANK_ROOT/src/simulation/StatisticManager.cpp \
    $$BANK_ROOT/src/database/Database.cpp \
    $$BANK_ROOT/src/database/sqlite/sqlite3.c
