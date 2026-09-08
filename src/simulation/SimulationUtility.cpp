#include "simulation/SimulationUtility.h"

#include <random>      // Génération aléatoire
#include <stdexcept>   // Gestion des erreurs
#include <iostream>    // Affichage console

#include "bank/Bank.h"
#include "client/AbstractOperation.h"
#include "client/Consultation.h"
#include "client/Transfer.h"
#include "client/Withdraw.h"

namespace
{
std::mt19937& generator()
{
    // Générateur aléatoire utilisé par les fonctions ci-dessous
    static std::mt19937 engine(std::random_device{}());
    return engine;
}
}

int SimulationUtility::getRandomServiceTime(int min, int max)
{
    // Vérifie que les bornes sont cohérentes
    if (min > max)
    {
        throw std::invalid_argument("La duree minimale est superieure a la duree maximale");
    }

    // Génère un temps de service aléatoire entre min et max
    return std::uniform_int_distribution<int>(min, max)(generator());
}

void SimulationUtility::printServiceTimeTrace(int currentSystemTime, int serviceTime)
{
    (void)currentSystemTime;
    // Affiche la durée du service
    std::cout << "Debut du service pour " << serviceTime << " unites de temps." << std::endl;
}

void SimulationUtility::printClientArrival(int currentSystemTime, bool served)
{
    (void)currentSystemTime;

    // Affiche si le client est servi directement ou mis en file
    if (served)
    {
        std::cout << "Un nouveau client arrive et est servi immediatement." << std::endl;
    }
    else
    {
        std::cout << "Un nouveau client arrive et rejoint la file d'attente." << std::endl;
    }
}

void SimulationUtility::printClientDeparture(int currentSystemTime)
{
    (void)currentSystemTime;
    // Affiche le départ d'un client après son service
    std::cout << "Un client part apres son service." << std::endl;
}

void SimulationUtility::printClientDepartureWithoutBeingServed(int currentSystemTime)
{
    (void)currentSystemTime;
    // Affiche le départ d'un client non servi
    std::cout << "Un client part sans etre servi." << std::endl;
}

void SimulationUtility::printBankStat(int currentSystemTime, const Bank& bank)
{
    // Affiche l'état de la banque à chaque unité de temps
    std::cout << "########### Au temps : " << currentSystemTime << " ##########" << std::endl;
    std::cout << bank.toString() << std::endl;
}

bool SimulationUtility::isPriorityClient(double priorityClientRate)
{
    // Détermine aléatoirement si le client est VIP
    return std::bernoulli_distribution(priorityClientRate)(generator());
}

AbstractOperation* SimulationUtility::getRandomOperation(int serviceTime)
{
    // Choisit aléatoirement une des trois opérations
    switch (std::uniform_int_distribution<int>(1, 3)(generator()))
    {
        case 1: return new Consultation(serviceTime);
        case 2: return new Transfer(serviceTime);
        default: return new Withdraw(serviceTime);
    }
}