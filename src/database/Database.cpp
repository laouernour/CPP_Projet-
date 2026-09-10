#include "database/Database.h"

#include <functional>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "sqlite3.h"

#include "client/AbstractClient.h"
#include "client/AbstractOperation.h"
#include "simulation/SimulationEntry.h"
#include "simulation/StatisticManager.h"

namespace
{
// Schéma de la base (identique à database/database.sql).
const char* const SCHEMA = R"sql(
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS simulations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    duration INTEGER NOT NULL CHECK (duration >= 0),
    cashier_count INTEGER NOT NULL CHECK (cashier_count > 0),
    min_service_time INTEGER NOT NULL CHECK (min_service_time > 0),
    max_service_time INTEGER NOT NULL CHECK (max_service_time >= min_service_time),
    client_arrival_interval INTEGER NOT NULL CHECK (client_arrival_interval > 0),
    priority_client_rate REAL NOT NULL CHECK (priority_client_rate BETWEEN 0 AND 1),
    client_patience_time INTEGER NOT NULL CHECK (client_patience_time >= 0),
    started_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS operations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    type TEXT NOT NULL CHECK (type IN ('consultation', 'transfer', 'withdraw')),
    service_time INTEGER NOT NULL CHECK (service_time > 0),
    is_urgent INTEGER NOT NULL DEFAULT 0 CHECK (is_urgent IN (0, 1))
);

CREATE TABLE IF NOT EXISTS clients (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    simulation_id INTEGER NOT NULL,
    operation_id INTEGER NOT NULL,
    client_type TEXT NOT NULL CHECK (client_type IN ('standard', 'vip')),
    arrival_time INTEGER NOT NULL CHECK (arrival_time >= 0),
    patience_time INTEGER NOT NULL CHECK (patience_time >= 0),
    service_start_time INTEGER,
    departure_time INTEGER,
    served INTEGER NOT NULL DEFAULT 0 CHECK (served IN (0, 1)),
    FOREIGN KEY (simulation_id) REFERENCES simulations(id) ON DELETE CASCADE,
    FOREIGN KEY (operation_id) REFERENCES operations(id),
    CHECK (service_start_time IS NULL OR service_start_time >= arrival_time),
    CHECK (departure_time IS NULL OR departure_time >= arrival_time)
);

CREATE TABLE IF NOT EXISTS statistics (
    simulation_id INTEGER PRIMARY KEY,
    served_client_count INTEGER NOT NULL CHECK (served_client_count >= 0),
    non_served_client_count INTEGER NOT NULL CHECK (non_served_client_count >= 0),
    average_waiting_time REAL NOT NULL,
    average_service_time REAL NOT NULL,
    cashier_occupation_rate REAL NOT NULL,
    client_satisfaction_rate REAL NOT NULL,
    FOREIGN KEY (simulation_id) REFERENCES simulations(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_clients_simulation_id ON clients(simulation_id);
)sql";

// Prépare une requête, exécute une fonction de liaison, puis fait un seul sqlite3_step.
// Utilisé pour les INSERT.
void runInsert(sqlite3* db, const char* sql,
               const std::function<void(sqlite3_stmt*)>& bind)
{
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error(std::string("SQL prepare: ") + sqlite3_errmsg(db));
    }
    bind(stmt);
    const int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
    {
        throw std::runtime_error(std::string("SQL step: ") + sqlite3_errmsg(db));
    }
}

// Enregistre l'opération d'un client et renvoie son id.
long long insertOperation(sqlite3* db, const AbstractClient* client)
{
    const AbstractOperation* op = client->getOperation();
    runInsert(db,
        "INSERT INTO operations (type, service_time, is_urgent) VALUES (?, ?, ?);",
        [&](sqlite3_stmt* s)
        {
            const std::string type = op->typeName();
            sqlite3_bind_text(s, 1, type.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(s, 2, op->getServiceTime());
            sqlite3_bind_int(s, 3, op->isUrgent() ? 1 : 0);
        });
    return sqlite3_last_insert_rowid(db);
}

// Enregistre un client rattaché à une simulation.
void insertClient(sqlite3* db, long long simulationId, long long operationId,
                  const AbstractClient* client, bool served)
{
    runInsert(db,
        "INSERT INTO clients (simulation_id, operation_id, client_type, arrival_time, "
        "patience_time, service_start_time, departure_time, served) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);",
        [&](sqlite3_stmt* s)
        {
            sqlite3_bind_int64(s, 1, simulationId);
            sqlite3_bind_int64(s, 2, operationId);
            sqlite3_bind_text(s, 3, client->isPriority() ? "vip" : "standard", -1, SQLITE_STATIC);
            sqlite3_bind_int(s, 4, client->getArrivalTime());
            // La patience initiale n'est plus connue ici : on enregistre 0 (valeur finale).
            sqlite3_bind_int(s, 5, 0);
            if (served)
            {
                sqlite3_bind_int(s, 6, client->getServiceStartTime());
            }
            else
            {
                sqlite3_bind_null(s, 6);   // Client jamais servi
            }
            sqlite3_bind_int(s, 7, client->getDepartureTime());
            sqlite3_bind_int(s, 8, served ? 1 : 0);
        });
}
}   // namespace

Database::Database(const std::string& path)
    : m_db(nullptr)
{
    if (sqlite3_open(path.c_str(), &m_db) != SQLITE_OK)
    {
        const std::string msg = m_db ? sqlite3_errmsg(m_db) : "impossible d'ouvrir la base";
        sqlite3_close(m_db);
        m_db = nullptr;
        throw std::runtime_error("Database: " + msg);
    }
    exec("PRAGMA foreign_keys = ON;");
    exec(SCHEMA);
}

Database::~Database()
{
    sqlite3_close(m_db);
}

void Database::exec(const std::string& sql) const
{
    char* errMsg = nullptr;
    if (sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        const std::string msg = errMsg ? errMsg : "erreur SQL";
        sqlite3_free(errMsg);
        throw std::runtime_error("Database::exec: " + msg);
    }
}

long long Database::saveSimulation(const SimulationEntry& entry, const StatisticManager& stats)
{
    exec("BEGIN;");
    try
    {
        // 1) Les paramètres de la simulation
        runInsert(m_db,
            "INSERT INTO simulations (duration, cashier_count, min_service_time, "
            "max_service_time, client_arrival_interval, priority_client_rate, "
            "client_patience_time) VALUES (?, ?, ?, ?, ?, ?, ?);",
            [&](sqlite3_stmt* s)
            {
                sqlite3_bind_int(s, 1, entry.getSimulationDuration());
                sqlite3_bind_int(s, 2, entry.getCashierCount());
                sqlite3_bind_int(s, 3, entry.getMinServiceTime());
                sqlite3_bind_int(s, 4, entry.getMaxServiceTime());
                sqlite3_bind_int(s, 5, entry.getClientArrivalInterval());
                sqlite3_bind_double(s, 6, entry.getPriorityClientRate());
                sqlite3_bind_int(s, 7, entry.getClientPatienceTime());
            });
        const long long simulationId = sqlite3_last_insert_rowid(m_db);

        // 2) Les statistiques calculées
        runInsert(m_db,
            "INSERT INTO statistics (simulation_id, served_client_count, "
            "non_served_client_count, average_waiting_time, average_service_time, "
            "cashier_occupation_rate, client_satisfaction_rate) VALUES (?, ?, ?, ?, ?, ?, ?);",
            [&](sqlite3_stmt* s)
            {
                sqlite3_bind_int64(s, 1, simulationId);
                sqlite3_bind_int(s, 2, stats.servedClientCount());
                sqlite3_bind_int(s, 3, stats.nonServedClientCount());
                sqlite3_bind_double(s, 4, stats.calculateAverageClientWaitingTime());
                sqlite3_bind_double(s, 5, stats.calculateAverageClientServiceTime());
                sqlite3_bind_double(s, 6, stats.calculateAverageCashierOccupationRate(entry.getCashierCount()));
                sqlite3_bind_double(s, 7, stats.calculateClientSatisfactionRate());
            });

        // 3) Le détail des clients (servis et non servis)
        for (const AbstractClient* client : stats.servedClients())
        {
            const long long operationId = insertOperation(m_db, client);
            insertClient(m_db, simulationId, operationId, client, true);
        }
        for (const AbstractClient* client : stats.nonServedClients())
        {
            const long long operationId = insertOperation(m_db, client);
            insertClient(m_db, simulationId, operationId, client, false);
        }

        exec("COMMIT;");
        return simulationId;
    }
    catch (...)
    {
        exec("ROLLBACK;");
        throw;
    }
}

std::string Database::readAllSimulations() const
{
    const char* sql =
        "SELECT s.id, s.started_at, s.duration, s.cashier_count, "
        "st.served_client_count, st.non_served_client_count, "
        "st.average_waiting_time, st.average_service_time, "
        "st.cashier_occupation_rate, st.client_satisfaction_rate "
        "FROM simulations s LEFT JOIN statistics st ON st.simulation_id = s.id "
        "ORDER BY s.id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error(std::string("SQL prepare: ") + sqlite3_errmsg(m_db));
    }

    std::ostringstream out;
    out << "===================== Simulations enregistrees =====================\n";
    out << std::left
        << std::setw(4)  << "id"
        << std::setw(21) << "date"
        << std::setw(7)  << "duree"
        << std::setw(6)  << "cais."
        << std::setw(8)  << "servis"
        << std::setw(10) << "n.servis"
        << std::setw(9)  << "attente"
        << std::setw(9)  << "service"
        << std::setw(9)  << "occ.%"
        << std::setw(9)  << "satis.%"
        << "\n";
    out << std::string(90, '-') << "\n";
    out << std::fixed << std::setprecision(2);

    int rows = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ++rows;
        const unsigned char* date = sqlite3_column_text(stmt, 1);
        out << std::left
            << std::setw(4)  << sqlite3_column_int(stmt, 0)
            << std::setw(21) << (date ? reinterpret_cast<const char*>(date) : "")
            << std::setw(7)  << sqlite3_column_int(stmt, 2)
            << std::setw(6)  << sqlite3_column_int(stmt, 3)
            << std::setw(8)  << sqlite3_column_int(stmt, 4)
            << std::setw(10) << sqlite3_column_int(stmt, 5)
            << std::setw(9)  << sqlite3_column_double(stmt, 6)
            << std::setw(9)  << sqlite3_column_double(stmt, 7)
            << std::setw(9)  << sqlite3_column_double(stmt, 8)
            << std::setw(9)  << sqlite3_column_double(stmt, 9)
            << "\n";
    }
    sqlite3_finalize(stmt);

    if (rows == 0)
    {
        out << "(aucune simulation enregistree)\n";
    }
    out << "===================================================================";
    return out.str();
}
