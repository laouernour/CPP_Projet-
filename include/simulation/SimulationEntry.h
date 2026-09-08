#ifndef SIMULATIONENTRY_H
#define SIMULATIONENTRY_H

/** Parametres immuables utilises pour une execution de simulation. */
class SimulationEntry
{
public:
    SimulationEntry(int simulationDuration, int cashierCount, int minServiceTime,
                    int maxServiceTime, int clientArrivalInterval,
                    double priorityClientRate, int clientPatienceTime);

    int getSimulationDuration() const;
    int getCashierCount() const;
    int getMinServiceTime() const;
    int getMaxServiceTime() const;
    int getClientArrivalInterval() const;
    double getPriorityClientRate() const;
    int getClientPatienceTime() const;

private:
    int m_simulationDuration;
    int m_cashierCount;
    int m_minServiceTime;
    int m_maxServiceTime;
    int m_clientArrivalInterval;
    double m_priorityClientRate;
    int m_clientPatienceTime;
};

#endif
