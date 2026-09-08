#include "simulation/StatisticManager.h"

#include "client/AbstractClient.h"

void StatisticManager::registerServedClient(AbstractClient* client)
{
    // Ajoute un client servi dans la liste des clients servis
    m_servedClients.push_back(client);
}

void StatisticManager::registerNonServedClient(AbstractClient* client)
{
    // Ajoute un client non servi dans la liste correspondante
    m_nonServedClients.push_back(client);
}

void StatisticManager::simulationDurationRecord()
{
    // Compte une unité de temps de simulation
    ++m_simulationDuration;
}

void StatisticManager::cashierOccupationRecord()
{
    // Compte une unité de temps pendant laquelle un caissier est occupé
    ++m_occupiedCashier;
}

double StatisticManager::calculateAverageCashierOccupationRate(int cashierCount) const
{
    // Évite une division invalide
    if (cashierCount <= 0 || m_simulationDuration == 0)
    {
        return 0.0;
    }

    // Calcule le taux moyen d'occupation des caissiers
    return 100.0 * m_occupiedCashier / (m_simulationDuration * cashierCount);
}

double StatisticManager::calculateAverageClientWaitingTime() const
{
    // Aucun client servi signifie aucun temps d'attente à calculer
    if (m_servedClients.empty())
    {
        return 0.0;
    }

    int totalWaitingTime = 0;

    for (const AbstractClient* client : m_servedClients)
    {
        // Attente = début du service - arrivée
        totalWaitingTime += client->getServiceStartTime() - client->getArrivalTime();
    }

    // Calcule la moyenne d'attente
    return static_cast<double>(totalWaitingTime) / m_servedClients.size();
}

double StatisticManager::calculateAverageClientServiceTime() const
{
    // Aucun client servi signifie aucun temps de service à calculer
    if (m_servedClients.empty())
    {
        return 0.0;
    }

    int totalServiceTime = 0;

    for (const AbstractClient* client : m_servedClients)
    {
        // Temps de service = départ - début du service
        totalServiceTime += client->getDepartureTime() - client->getServiceStartTime();
    }

    // Calcule la moyenne du temps de service
    return static_cast<double>(totalServiceTime) / m_servedClients.size();
}

int StatisticManager::servedClientCount() const
{
    // Retourne le nombre de clients servis
    return static_cast<int>(m_servedClients.size());
}

int StatisticManager::nonServedClientCount() const
{
    // Retourne le nombre de clients non servis
    return static_cast<int>(m_nonServedClients.size());
}

double StatisticManager::calculateClientSatisfactionRate() const
{
    // Nombre total de clients
    const int totalClientCount = servedClientCount() + nonServedClientCount();

    if (totalClientCount == 0)
    {
        return 0.0;
    }

    // Calcule le pourcentage de clients servis
    return 100.0 * servedClientCount() / totalClientCount;
}