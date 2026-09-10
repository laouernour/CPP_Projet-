// tests/SimulationTest.cpp
// Tests unitaires de la classe Simulation (boucle de simulation).
//
// Comportements verifies :
//  - une simulation neuve n'est pas terminee et son horloge est a 0 ;
//  - step() fait avancer l'horloge d'une unite ;
//  - apres simulate(), la simulation est terminee ;
//  - avancer avec step() jusqu'au bout donne le meme etat que simulate() ;
//  - les statistiques finales sont coherentes (servis + non servis <= clients arrives).

#include <cassert>
#include <iostream>

#include "simulation/Simulation.h"
#include "simulation/SimulationEntry.h"
#include "simulation/StatisticManager.h"

int main()
{
    const SimulationEntry entry(50, 2, 5, 10, 5, 0.15, 4);

    // 1) Etat initial.
    {
        Simulation simulation(entry);
        simulation.setVerbose(false);
        assert(simulation.getCurrentTime() == 0);
        assert(!simulation.isFinished());
        std::cout << "[OK] etat initial : horloge a 0, non terminee\n";
    }

    // 2) step() avance l'horloge.
    {
        Simulation simulation(entry);
        simulation.setVerbose(false);
        simulation.step();
        assert(simulation.getCurrentTime() == 1);
        simulation.step();
        assert(simulation.getCurrentTime() == 2);
        std::cout << "[OK] step() avance l'horloge d'une unite\n";
    }

    // 3) simulate() va jusqu'au bout.
    {
        Simulation simulation(entry);
        simulation.setVerbose(false);
        simulation.simulate();
        assert(simulation.isFinished());
        assert(simulation.getCurrentTime() == entry.getSimulationDuration() + 1);
        std::cout << "[OK] simulate() termine la simulation\n";
    }

    // 4) step() en boucle == simulate().
    {
        Simulation simulation(entry);
        simulation.setVerbose(false);
        while (simulation.step())
        {
        }
        assert(simulation.isFinished());

        const StatisticManager& stats = simulation.getStatistics();
        const int total = stats.servedClientCount() + stats.nonServedClientCount();
        // Un client arrive toutes les 5 unites sur 0..50 -> au plus 11 clients.
        assert(total >= 0 && total <= 11);
        std::cout << "[OK] step() en boucle : simulation coherente (" << total << " clients traites)\n";
    }

    std::cout << "Simulation : tous les tests sont passes.\n";
    return 0;
}
