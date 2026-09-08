#ifndef SIMULATIONENTRY_H
#define SIMULATIONENTRY_H

class SimulationEntry
{
public:
    SimulationEntry(int simulationDuration, int cashierCount, int minServiceTime,
                    int maxServiceTime, int clientArrivalInterval,
                    double priorityClientRate, int clientPatienceTime);
    // Crée les paramètres de la simulation

    int getSimulationDuration() const;       // Récupère la durée
    int getCashierCount() const;              // Récupère le nombre de caissiers
    int getMinServiceTime() const;            // Récupère le temps minimum
    int getMaxServiceTime() const;            // Récupère le temps maximum
    int getClientArrivalInterval() const;     // Récupère l'intervalle d'arrivée
    double getPriorityClientRate() const;     // Récupère le taux de VIP
    int getClientPatienceTime() const;        // Récupère la patience des clients

private:
    int m_simulationDuration;      // Durée de la simulation
    int m_cashierCount;            // Nombre de caissiers
    int m_minServiceTime;          // Temps de service minimum
    int m_maxServiceTime;          // Temps de service maximum
    int m_clientArrivalInterval;   // Intervalle entre les arrivées
    double m_priorityClientRate;   // Pourcentage de clients VIP
    int m_clientPatienceTime;      // Temps de patience
};

#endif