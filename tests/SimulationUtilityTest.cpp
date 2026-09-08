// tests/SimulationUtilityTest.cpp
// Tests unitaires de SimulationUtility (fonctions aleatoires de la simulation).
//
// Les fonctions sont aleatoires : on les appelle un grand nombre de fois et on
// verifie des PROPRIETES qui doivent toujours etre vraies (bornes, valeurs possibles).
//
// Comportements verifies :
//  - getRandomServiceTime(min, max) reste dans [min, max] ; rejette min > max ;
//  - isPriorityClient(taux) : 0.0 -> jamais VIP, 1.0 -> toujours VIP, 0.5 -> les deux ;
//  - getRandomOperation() peut generer les trois types et conserve le temps de service.

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

#include "simulation/SimulationUtility.h"
#include "client/AbstractOperation.h"

int main()
{
    const int iterations = 5000;

    // 1) getRandomServiceTime : le resultat est toujours dans l'intervalle demande.
    for (int i = 0; i < iterations; ++i)
    {
        int t = SimulationUtility::getRandomServiceTime(10, 20);
        assert(t >= 10 && t <= 20);
    }
    assert(SimulationUtility::getRandomServiceTime(7, 7) == 7); // min == max : une seule valeur
    std::cout << "[OK] getRandomServiceTime respecte min/max\n";

    // 1bis) min > max : la fonction leve std::invalid_argument.
    bool leveException = false;
    try { SimulationUtility::getRandomServiceTime(20, 10); }
    catch (const std::invalid_argument&) { leveException = true; }
    assert(leveException);
    std::cout << "[OK] getRandomServiceTime rejette min > max\n";

    // 2) isPriorityClient : bornes 0.0 et 1.0 deterministes.
    for (int i = 0; i < iterations; ++i)
    {
        assert(SimulationUtility::isPriorityClient(0.0) == false);
        assert(SimulationUtility::isPriorityClient(1.0) == true);
    }
    // Taux intermediaire : on doit rencontrer au moins un VIP et un non-VIP.
    bool vuVIP = false;
    bool vuNonVIP = false;
    for (int i = 0; i < iterations && !(vuVIP && vuNonVIP); ++i)
    {
        if (SimulationUtility::isPriorityClient(0.5)) vuVIP = true;
        else vuNonVIP = true;
    }
    assert(vuVIP && vuNonVIP);
    std::cout << "[OK] isPriorityClient respecte le taux VIP\n";

    // 3) getRandomOperation : les trois types apparaissent, et le temps de service est conserve.
    bool vuConsultation = false;
    bool vuVirement = false;
    bool vuRetrait = false;
    for (int i = 0; i < iterations; ++i)
    {
        AbstractOperation* op = SimulationUtility::getRandomOperation(13);
        assert(op->getServiceTime() == 13);

        const std::string nom = op->toString();
        if (nom == "Operation : Consultation") vuConsultation = true;
        else if (nom == "Operation : Virement") vuVirement = true;
        else if (nom == "Operation : Retrait")  vuRetrait = true;

        delete op; // ici aucune classe ne "possede" l'operation : le test la detruit
    }
    assert(vuConsultation && vuVirement && vuRetrait);
    std::cout << "[OK] getRandomOperation genere les 3 types\n";

    std::cout << "SimulationUtility : tous les tests sont passes.\n";
    return 0;
}
