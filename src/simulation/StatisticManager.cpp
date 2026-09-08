#include "simulation/StatisticManager.h"

#include "client/AbstractClient.h"

void StatisticManager::registerServedClient(AbstractClient* client)
{
    m_servedClients.push_back(client);
}

void StatisticManager::registerNonServedClient(AbstractClient* client)
{
    m_nonServedClients.push_back(client);
}

void StatisticManager::simulationDurationRecord()
{
    ++m_simulationDuration;
}

void StatisticManager::cashierOccupationRecord()
{
    ++m_occupiedCashier;
}

double StatisticManager::calculateAverageCashierOccupationRate(int cashierCount) const
{
    if (cashierCount <= 0 || m_simulationDuration == 0)
    {
        return 0.0;
    }
    return 100.0 * m_occupiedCashier / (m_simulationDuration * cashierCount);
}

double StatisticManager::calculateAverageClientWaitingTime() const
{
    if (m_servedClients.empty())
    {
        return 0.0;
    }
    int totalWaitingTime = 0;
    for (const AbstractClient* client : m_servedClients)
    {
        totalWaitingTime += client->getServiceStartTime() - client->getArrivalTime();
    }
    return static_cast<double>(totalWaitingTime) / m_servedClients.size();
}

double StatisticManager::calculateAverageClientServiceTime() const
{
    if (m_servedClients.empty())
    {
        return 0.0;
    }
    int totalServiceTime = 0;
    for (const AbstractClient* client : m_servedClients)
    {
        totalServiceTime += client->getDepartureTime() - client->getServiceStartTime();
    }
    return static_cast<double>(totalServiceTime) / m_servedClients.size();
}

int StatisticManager::servedClientCount() const
{
    return static_cast<int>(m_servedClients.size());
}

int StatisticManager::nonServedClientCount() const
{
    return static_cast<int>(m_nonServedClients.size());
}

double StatisticManager::calculateClientSatisfactionRate() const
{
    const int totalClientCount = servedClientCount() + nonServedClientCount();
    if (totalClientCount == 0)
    {
        return 0.0;
    }
    return 100.0 * servedClientCount() / totalClientCount;
}
