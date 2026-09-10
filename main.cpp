#include <iostream>

#include "simulation/Simulation.h"
#include "simulation/SimulationDefaults.h"
#include "database/Database.h"

namespace
{
// Enregistre les résultats de la simulation dans la base et affiche l'historique.
void persistAndShow(const Simulation& simulation)
{
    try
    {
        Database database("bank2.db");
        const long long id =
            database.saveSimulation(simulation.getEntry(), simulation.getStatistics());
        std::cout << "\nResultats enregistres dans bank2.db (simulation #" << id << ").\n\n";
        std::cout << database.readAllSimulations() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nErreur base de donnees : " << e.what() << '\n';
    }
}
}

int main()
{
    // Création des paramètres de la simulation
    const SimulationEntry parameters = defaultSimulationEntry();

    Simulation simulation(parameters);

    // Mode console : affiche les paramètres, déroule la simulation, puis les résultats.
    std::cout << "========================================\n";
    std::cout << "       SIMULATION D'AGENCE BANCAIRE\n";
    std::cout << "========================================\n";
    std::cout << "Duree                 : " << parameters.getSimulationDuration() << " unites\n";
    std::cout << "Nombre de caissiers   : " << parameters.getCashierCount() << "\n";
    std::cout << "Duree d'un service    : " << parameters.getMinServiceTime()
              << " a " << parameters.getMaxServiceTime() << " unites\n";
    std::cout << "Arrivee d'un client   : toutes les "
              << parameters.getClientArrivalInterval() << " unites\n";
    std::cout << "Taux de clients VIP   : " << parameters.getPriorityClientRate() * 100 << " %\n";
    std::cout << "Patience              : " << parameters.getClientPatienceTime() << " unites\n\n";

    simulation.simulate();

    std::cout << "Simulation terminee.\n\n";
    std::cout << simulation.simulationResults() << std::endl;

    persistAndShow(simulation);
    return 0;
}
