// tests/StatisticManagerTest.cpp
// Tests unitaires de StatisticManager (calcul des statistiques finales).
//
// On prepare des clients avec des temps CONNUS (arrivee / debut de service / depart)
// pour pouvoir predire exactement les moyennes calculees.
//
// Comportements verifies :
//  - etat initial neutre (aucune division par zero) ;
//  - registerServedClient / servedClientCount ;
//  - registerNonServedClient / nonServedClientCount ;
//  - temps d'attente moyen = moyenne (debut de service - arrivee) ;
//  - temps de service moyen = moyenne (depart - debut de service) ;
//  - taux de satisfaction = servis / (servis + non servis) ;
//  - taux d'occupation des caissiers.

#include <cassert>
#include <cmath>
#include <iostream>

#include "simulation/StatisticManager.h"
#include "client/Client.h"
#include "client/Consultation.h"

// Comparaison de deux flottants avec une petite tolerance.
static bool presque(double a, double b) { return std::fabs(a - b) < 1e-6; }

int main()
{
    StatisticManager stats;

    // 1) Etat initial : rien enregistre, tous les taux valent 0 (pas de division par zero).
    assert(stats.servedClientCount() == 0);
    assert(stats.nonServedClientCount() == 0);
    assert(presque(stats.calculateClientSatisfactionRate(), 0.0));
    assert(presque(stats.calculateAverageClientWaitingTime(), 0.0));
    assert(presque(stats.calculateAverageClientServiceTime(), 0.0));
    assert(presque(stats.calculateAverageCashierOccupationRate(3), 0.0));
    std::cout << "[OK] etat initial neutre\n";

    // Client A : arrive a 0, service commence a 5, part a 15  -> attente 5, service 10
    Client a(0, new Consultation(10), 9);
    a.setServiceStartTime(5);
    a.setDepartureTime(15);
    // Client B : arrive a 10, service commence a 12, part a 20 -> attente 2, service 8
    Client b(10, new Consultation(8), 9);
    b.setServiceStartTime(12);
    b.setDepartureTime(20);

    // 2) enregistrement de clients servis + comptage.
    stats.registerServedClient(&a);
    stats.registerServedClient(&b);
    assert(stats.servedClientCount() == 2);
    std::cout << "[OK] enregistrement + comptage clients servis\n";

    // 3) enregistrement d'un client non servi + comptage.
    Client c(0, new Consultation(5), 0);
    stats.registerNonServedClient(&c);
    assert(stats.nonServedClientCount() == 1);
    std::cout << "[OK] enregistrement + comptage clients non servis\n";

    // 4) temps d'attente moyen = ((5 - 0) + (12 - 10)) / 2 = 3.5
    assert(presque(stats.calculateAverageClientWaitingTime(), 3.5));
    std::cout << "[OK] temps d'attente moyen\n";

    // 5) temps de service moyen = ((15 - 5) + (20 - 12)) / 2 = 9.0
    assert(presque(stats.calculateAverageClientServiceTime(), 9.0));
    std::cout << "[OK] temps de service moyen\n";

    // 6) taux de satisfaction = 100 * 2 / (2 + 1) = 66.666...
    assert(presque(stats.calculateClientSatisfactionRate(), 200.0 / 3.0));
    std::cout << "[OK] taux de satisfaction\n";

    // 7) taux d'occupation : 10 unites de simulation, 15 "unites-caissier" occupees, 3 caissiers
    //    -> 100 * 15 / (10 * 3) = 50 %
    for (int i = 0; i < 10; ++i) stats.simulationDurationRecord();
    for (int i = 0; i < 15; ++i) stats.cashierOccupationRecord();
    assert(presque(stats.calculateAverageCashierOccupationRate(3), 50.0));
    std::cout << "[OK] taux d'occupation des caissiers\n";

    std::cout << "StatisticManager : tous les tests sont passes.\n";
    return 0;
}
