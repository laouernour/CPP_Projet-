#ifndef STATISTICMANAGER_H

#define STATISTICMANAGER_H

#include <vector>   // Permet d'utiliser vector

class AbstractClient;   // On utilise la classe AbstractClient

class StatisticManager
{
public:
    void registerServedClient(AbstractClient* client);   // Enregistre un client servi
    void registerNonServedClient(AbstractClient* client); // Enregistre un client non servi
    void simulationDurationRecord();                     // Enregistre la durée
    void cashierOccupationRecord();                      // Enregistre l'occupation des caissiers

    double calculateAverageCashierOccupationRate(int cashierCount) const; // Calcule le taux d'occupation
    double calculateAverageClientWaitingTime() const;                     // Calcule l'attente moyenne
    double calculateAverageClientServiceTime() const;                     // Calcule le service moyen
    int servedClientCount() const;                                        // Compte les clients servis
    int nonServedClientCount() const;                                     // Compte les clients non servis
    double calculateClientSatisfactionRate() const;                       // Calcule la satisfaction

    const std::vector<AbstractClient*>& servedClients() const;    // Liste des clients servis
    const std::vector<AbstractClient*>& nonServedClients() const; // Liste des clients non servis

private:
    std::vector<AbstractClient*> m_servedClients;      // Liste des clients servis
    std::vector<AbstractClient*> m_nonServedClients;   // Liste des clients non servis

    int m_simulationDuration = 0;   // Durée de la simulation
    int m_occupiedCashier = 0;      // Nombre de fois où les caissiers sont occupés
};

#endif