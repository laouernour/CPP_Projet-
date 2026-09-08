#ifndef SIMULATIONUTILITY_H
#define SIMULATIONUTILITY_H

class AbstractOperation;
class Bank;

/** Fonctions aleatoires necessaires a la simulation. */
class SimulationUtility
{
public:
    static int getRandomServiceTime(int min, int max);
    static void printServiceTimeTrace(int currentSystemTime, int serviceTime);
    static void printClientArrival(int currentSystemTime, bool served);
    static void printClientDeparture(int currentSystemTime);
    static void printClientDepartureWithoutBeingServed(int currentSystemTime);
    static void printBankStat(int currentSystemTime, const Bank& bank);
    static bool isPriorityClient(double priorityClientRate);
    static AbstractOperation* getRandomOperation(int serviceTime);
};

#endif
