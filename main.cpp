#include <cstring>
#include <iostream>

#include "simulation/Simulation.h"
#include "database/Database.h"
#include "ui/RealtimeView.h"

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

int main(int argc, char** argv)
{
    // "gui" en argument -> fenêtre graphique temps réel ; sinon -> mode console
    const bool guiMode = (argc > 1 && std::strcmp(argv[1], "gui") == 0);

    // Création des paramètres de la simulation
    const SimulationEntry parameters(
        100,  // duree de la simulation
        3,    // nombre de caissiers
        10,   // temps minimal de service
        20,   // temps maximal de service
        5,    // un client arrive toutes les 5 unites de temps
        0.10, // 10 % de clients VIP
        7);   // patience des clients non urgents

    Simulation simulation(parameters);

    if (guiMode)
    {
        // Mode graphique : la fenêtre fait avancer la simulation en temps réel
        RealtimeView view(simulation, 150);
        view.run();

        if (simulation.isFinished())
        {
            std::cout << simulation.simulationResults() << std::endl;
            persistAndShow(simulation);
        }
        else
        {
            std::cout << "Simulation interrompue (fenetre fermee avant la fin).\n";
        }
        return 0;
    }

    // Mode console : affiche les paramètres, déroule la simulation, puis les résultats
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
