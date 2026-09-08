#ifndef SIMULATION_H
#define SIMULATION_H

#include <string>
#include <vector>

#include "bank/Bank.h"
#include "simulation/SimulationEntry.h"
#include "simulation/StatisticManager.h"

class AbstractClient;
class Cashier;

/** Orchestre les arrivees, les services, la file et les statistiques. */
class Simulation
{
public:
    explicit Simulation(const SimulationEntry& simulationEntry);
    ~Simulation();

    Simulation(const Simulation&) = delete;
    Simulation& operator=(const Simulation&) = delete;

    void simulate();
    std::string simulationResults() const;

private:
    void updateBank(int currentSystemTime);
    void serveClient(int currentSystemTime, Cashier* cashier, AbstractClient* client);
    AbstractClient* nextWaitingClient();

    SimulationEntry m_simulationEntry;
    Bank m_bank;
    StatisticManager m_statisticManager;
    std::vector<AbstractClient*> m_clients;
};

#endif
