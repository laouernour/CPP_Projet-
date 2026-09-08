#include "simulation/SimulationEntry.h"

#include <stdexcept>

SimulationEntry::SimulationEntry(int simulationDuration, int cashierCount, int minServiceTime,
                                 int maxServiceTime, int clientArrivalInterval,
                                 double priorityClientRate, int clientPatienceTime)
    : m_simulationDuration(simulationDuration), m_cashierCount(cashierCount),
      m_minServiceTime(minServiceTime), m_maxServiceTime(maxServiceTime),
      m_clientArrivalInterval(clientArrivalInterval), m_priorityClientRate(priorityClientRate),
      m_clientPatienceTime(clientPatienceTime)
{
    if (simulationDuration < 0 || cashierCount <= 0 || minServiceTime <= 0 ||
        maxServiceTime < minServiceTime || clientArrivalInterval <= 0 ||
        priorityClientRate < 0.0 || priorityClientRate > 1.0 || clientPatienceTime < 0)
    {
        throw std::invalid_argument("Parametres de simulation invalides");
    }
}

int SimulationEntry::getSimulationDuration() const { return m_simulationDuration; }
int SimulationEntry::getCashierCount() const { return m_cashierCount; }
int SimulationEntry::getMinServiceTime() const { return m_minServiceTime; }
int SimulationEntry::getMaxServiceTime() const { return m_maxServiceTime; }
int SimulationEntry::getClientArrivalInterval() const { return m_clientArrivalInterval; }
double SimulationEntry::getPriorityClientRate() const { return m_priorityClientRate; }
int SimulationEntry::getClientPatienceTime() const { return m_clientPatienceTime; }
