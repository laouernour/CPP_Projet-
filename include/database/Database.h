#ifndef DATABASE_H
#define DATABASE_H

#include <string>

class SimulationEntry;     // Paramètres d'une simulation
class StatisticManager;    // Statistiques d'une simulation
struct sqlite3;            // Handle opaque de la bibliothèque SQLite

/**
 * Enregistre les résultats des simulations dans une base SQLite,
 * puis permet de les relire et de les afficher.
 *
 * Le schéma est décrit dans database/database.sql.
 */
class Database
{
public:
    explicit Database(const std::string& path);
    // Ouvre (ou crée) le fichier de base de données et crée les tables

    ~Database();
    // Ferme la base de données

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    // Une base de données ne se copie pas

    long long saveSimulation(const SimulationEntry& entry, const StatisticManager& stats);
    // Enregistre une exécution (paramètres + statistiques + clients). Retourne l'id de la ligne.

    std::string readAllSimulations() const;
    // Relit toutes les simulations enregistrées et retourne un tableau texte

private:
    sqlite3* m_db;   // Connexion à la base

    void exec(const std::string& sql) const;
    // Exécute une instruction SQL qui ne renvoie pas de résultat
};

#endif // DATABASE_H
