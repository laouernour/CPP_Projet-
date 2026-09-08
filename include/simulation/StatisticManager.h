#ifndef STATISTICMANAGER_H
#define STATISTICMANAGER_H

#include <vector>

class AbstractClient;

/** Accumule les resultats d'une simulation sans etre proprietaire des clients. */
class StatisticManager
{
public:
    void registerServedClient(AbstractClient* client);
    void registerNonServedClient(AbstractClient* client);
    void simulationDurationRecord();
    void cashierOccupationRecord();

    double calculateAverageCashierOccupationRate(int cashierCount) const;
    double calculateAverageClientWaitingTime() const;
    double calculateAverageClientServiceTime() const;
    int servedClientCount() const;
    int nonServedClientCount() const;
    double calculateClientSatisfactionRate() const;

private:
    std::vector<AbstractClient*> m_servedClients;
    std::vector<AbstractClient*> m_nonServedClients;
    int m_simulationDuration = 0;
    int m_occupiedCashier = 0;
};

#endif
