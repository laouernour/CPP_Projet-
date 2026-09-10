PRAGMA foreign_keys = ON;

-- Base SQLite pour la simulation bancaire.

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

CREATE TABLE IF NOT EXISTS cashiers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    simulation_id INTEGER NOT NULL,
    cashier_number INTEGER NOT NULL CHECK (cashier_number > 0),
    FOREIGN KEY (simulation_id) REFERENCES simulations(id) ON DELETE CASCADE,
    UNIQUE (simulation_id, cashier_number)
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

CREATE TABLE IF NOT EXISTS services (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    client_id INTEGER NOT NULL UNIQUE,
    cashier_id INTEGER NOT NULL,
    start_time INTEGER NOT NULL CHECK (start_time >= 0),
    end_time INTEGER,
    FOREIGN KEY (client_id) REFERENCES clients(id) ON DELETE CASCADE,
    FOREIGN KEY (cashier_id) REFERENCES cashiers(id),
    CHECK (end_time IS NULL OR end_time >= start_time)
);

-- Resultats statistiques calcules a la fin de chaque simulation.
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
CREATE INDEX IF NOT EXISTS idx_clients_arrival_time ON clients(arrival_time);
CREATE INDEX IF NOT EXISTS idx_services_cashier_id ON services(cashier_id);
