// tests/DatabaseTest.cpp
// Tests unitaires de la classe Database (persistance SQLite des simulations).
//
// Comportements verifies :
//  - une base neuve ne contient aucune simulation ;
//  - saveSimulation enregistre les parametres, les statistiques et les clients ;
//  - readAllSimulations relit les lignes enregistrees ;
//  - plusieurs simulations s'accumulent dans la base.

#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>

#include "database/Database.h"
#include "database/sqlite/sqlite3.h"

#include "simulation/Simulation.h"
#include "simulation/SimulationEntry.h"

namespace
{
int countRows(const std::string& path, const std::string& sql)
{
    sqlite3* db = nullptr;
    sqlite3_open(path.c_str(), &db);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    int value = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        value = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return value;
}
}

int main()
{
    const std::string path = "test_bank2.db";
    std::remove(path.c_str());   // repart d'une base propre

    // 1) Une base neuve ne contient aucune simulation.
    {
        Database db(path);
        const std::string dump = db.readAllSimulations();
        assert(dump.find("aucune simulation") != std::string::npos);
        std::cout << "[OK] base neuve vide\n";
    }

    // 2) saveSimulation enregistre une execution.
    long long firstId = 0;
    {
        SimulationEntry entry(60, 2, 5, 10, 5, 0.20, 4);
        Simulation simulation(entry);
        simulation.simulate();

        Database db(path);
        firstId = db.saveSimulation(simulation.getEntry(), simulation.getStatistics());
        assert(firstId == 1);

        const std::string dump = db.readAllSimulations();
        assert(dump.find("aucune simulation") == std::string::npos);
        std::cout << "[OK] enregistrement d'une simulation\n";
    }

    // 3) Les tables sont bien remplies.
    assert(countRows(path, "SELECT COUNT(*) FROM simulations;") == 1);
    assert(countRows(path, "SELECT COUNT(*) FROM statistics;") == 1);
    const int served = countRows(path, "SELECT served_client_count FROM statistics;");
    const int nonServed = countRows(path, "SELECT non_served_client_count FROM statistics;");
    assert(countRows(path, "SELECT COUNT(*) FROM clients;") == served + nonServed);
    assert(countRows(path, "SELECT COUNT(*) FROM operations;") == served + nonServed);
    assert(countRows(path,
        "SELECT COUNT(*) FROM operations WHERE type NOT IN ('consultation','transfer','withdraw');") == 0);
    std::cout << "[OK] tables simulations / statistics / clients / operations remplies\n";

    // 4) Une deuxieme simulation s'ajoute sans effacer la premiere.
    {
        SimulationEntry entry(60, 3, 5, 10, 5, 0.10, 4);
        Simulation simulation(entry);
        simulation.simulate();

        Database db(path);
        const long long secondId = db.saveSimulation(simulation.getEntry(), simulation.getStatistics());
        assert(secondId == 2);
    }
    assert(countRows(path, "SELECT COUNT(*) FROM simulations;") == 2);
    std::cout << "[OK] accumulation de plusieurs simulations\n";

    std::remove(path.c_str());
    std::cout << "Database : tous les tests sont passes.\n";
    return 0;
}
