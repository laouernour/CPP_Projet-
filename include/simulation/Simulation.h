#ifndef SIMULATION_H
#define SIMULATION_H

#include <string>                         // Pour utiliser string
#include <vector>                         // Pour utiliser vector

#include "bank/Bank.h"                    // La simulation utilise la banque
#include "simulation/SimulationEntry.h"   // Contient les paramètres
#include "simulation/StatisticManager.h"  // Gère les statistiques

class AbstractClient;   // On utilise la classe Client
class Cashier;          // On utilise la classe Cashier

class Simulation
{
public:
    explicit Simulation(const SimulationEntry& simulationEntry);
    // Crée la simulation avec ses paramètres

    ~Simulation();   // Détruit la simulation

    Simulation(const Simulation&) = delete;
    Simulation& operator=(const Simulation&) = delete;
    // Empêche de copier la simulation

    void simulate();   // Lance la simulation complète (mode console)

    bool step();       // Avance d'une seule unité de temps ; renvoie false quand c'est fini
    bool isFinished() const;      // Vrai quand la simulation est terminée
    int  getCurrentTime() const;  // Unité de temps courante

    void setVerbose(bool verbose);   // Active/désactive l'affichage console (off en mode graphique)

    std::string simulationResults() const;   // Retourne les résultats

    const SimulationEntry& getEntry() const;          // Paramètres utilisés
    const StatisticManager& getStatistics() const;    // Statistiques accumulées
    const Bank& getBank() const;                      // État de la banque (affichage)

private:
    void updateBank(int currentSystemTime);
    // Met à jour la banque à chaque unité de temps

    void serveClient(int currentSystemTime, Cashier* cashier, AbstractClient* client);
    // Donne un client à un caissier

    AbstractClient* nextWaitingClient();
    // Cherche le prochain client à servir

    SimulationEntry m_simulationEntry;       // Paramètres de la simulation
    Bank m_bank;                             // Banque
    StatisticManager m_statisticManager;     // Statistiques
    std::vector<AbstractClient*> m_clients;  // Liste des clients

    int m_currentSystemTime = 0;   // Unité de temps courante
    bool m_verbose = true;         // Affichage console de la trace
};

#endif
