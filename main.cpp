#include <iostream>
#include "simulation/Simulation.h"

#include <iostream>

int main()
{
    // Création des paramètres de la simulation
    const SimulationEntry parameters(
        1000, // duree de la simulation
        3,    // nombre de caissiers
        10,   // temps minimal de service
        20,   // temps maximal de service
        5,    // un client arrive toutes les 5 unites de temps
        0.10, // 10 % de clients VIP
        7);   // patience des clients non urgents

    // Affichage des paramètres utilisés
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

    // Création et lancement de la simulation
    Simulation simulation(parameters);
    simulation.simulate();

    // Affichage des résultats
    std::cout << "Simulation terminee.\n\n";
    std::cout << simulation.simulationResults() << std::endl;

    return 0;
}