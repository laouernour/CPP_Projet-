#ifndef SIMULATIONUTILITY_H
#define SIMULATIONUTILITY_H

class AbstractOperation;   // On utilise AbstractOperation
class Bank;                // On utilise Bank

class SimulationUtility
{
public:
    static int getRandomServiceTime(int min, int max);
    // Génère un temps de service aléatoire

    static void printServiceTimeTrace(int currentSystemTime, int serviceTime);
    // Affiche le temps de service

    static void printClientArrival(int currentSystemTime, bool served);
    // Affiche l'arrivée d'un client

    static void printClientDeparture(int currentSystemTime);
    // Affiche le départ d'un client servi

    static void printClientDepartureWithoutBeingServed(int currentSystemTime);
    // Affiche le départ d'un client non servi

    static void printBankStat(int currentSystemTime, const Bank& bank);
    // Affiche l'état de la banque

    static bool isPriorityClient(double priorityClientRate);
    // Détermine si le client est VIP

    static AbstractOperation* getRandomOperation(int serviceTime);
    // Choisit une opération aléatoire
};

#endif