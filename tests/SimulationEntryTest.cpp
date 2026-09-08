// tests/SimulationEntryTest.cpp
// Tests unitaires de la classe SimulationEntry (parametres d'une simulation).
//
// Comportements verifies :
//  - un jeu de parametres valides est accepte et les getters renvoient ces valeurs ;
//  - les cas limites acceptes (duree 0, patience 0) ;
//  - chaque parametre invalide provoque une exception std::invalid_argument.

#include <cassert>
#include <iostream>
#include <stdexcept>

#include "simulation/SimulationEntry.h"

// Renvoie true si le constructeur rejette (par exception) ce jeu de parametres.
static bool constructeurRejette(int duree, int caissiers, int minS, int maxS,
                                int intervalle, double tauxVIP, int patience)
{
    try
    {
        SimulationEntry entry(duree, caissiers, minS, maxS, intervalle, tauxVIP, patience);
        return false; // aucune exception : le parametre a ete accepte
    }
    catch (const std::invalid_argument&)
    {
        return true;
    }
}

int main()
{
    // 1) Parametres valides : aucune exception + getters coherents.
    SimulationEntry entry(1000, 3, 10, 20, 5, 0.10, 7);
    assert(entry.getSimulationDuration() == 1000);
    assert(entry.getCashierCount() == 3);
    assert(entry.getMinServiceTime() == 10);
    assert(entry.getMaxServiceTime() == 20);
    assert(entry.getClientArrivalInterval() == 5);
    assert(entry.getPriorityClientRate() == 0.10);
    assert(entry.getClientPatienceTime() == 7);
    std::cout << "[OK] parametres valides + getters\n";

    // 2) Cas limites acceptes : duree = 0 et patience = 0 sont valides.
    SimulationEntry limite(0, 1, 1, 1, 1, 0.0, 0);
    assert(limite.getSimulationDuration() == 0);
    assert(limite.getClientPatienceTime() == 0);
    std::cout << "[OK] cas limites acceptes (duree 0, patience 0)\n";

    // 3) Parametres invalides : chaque cas doit lever std::invalid_argument.
    assert(constructeurRejette(  -1, 3, 10, 20, 5, 0.1,  7)); // duree negative
    assert(constructeurRejette(1000, 0, 10, 20, 5, 0.1,  7)); // 0 caissier
    assert(constructeurRejette(1000, 3,  0, 20, 5, 0.1,  7)); // temps de service min nul
    assert(constructeurRejette(1000, 3, 20, 10, 5, 0.1,  7)); // max < min
    assert(constructeurRejette(1000, 3, 10, 20, 0, 0.1,  7)); // intervalle d'arrivee nul
    assert(constructeurRejette(1000, 3, 10, 20, 5, 1.5,  7)); // taux VIP > 1
    assert(constructeurRejette(1000, 3, 10, 20, 5, -0.1, 7)); // taux VIP < 0
    assert(constructeurRejette(1000, 3, 10, 20, 5, 0.1, -1)); // patience negative
    std::cout << "[OK] parametres invalides rejetes (8 cas)\n";

    std::cout << "SimulationEntry : tous les tests sont passes.\n";
    return 0;
}
