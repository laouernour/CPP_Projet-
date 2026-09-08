#include "simulation/Simulation.h"   // Utilise la classe Simulation

#include <iomanip>   // Pour formater les résultats
#include <sstream>    // Pour construire une chaîne de caractères

#include "bank/Cashier.h"                  // Utilise Cashier
#include "bank/Queue.h"                    // Utilise Queue
#include "client/AbstractClient.h"         // Utilise AbstractClient
#include "client/Client.h"                 // Utilise Client normal
#include "client/VIPClient.h"              // Utilise Client VIP
#include "simulation/SimulationUtility.h"  // Utilise les fonctions utiles


Simulation::Simulation(const SimulationEntry& simulationEntry)
    : m_simulationEntry(simulationEntry),          // Enregistre les paramètres
      m_bank(simulationEntry.getCashierCount())    // Crée la banque avec les caissiers
{
}


Simulation::~Simulation()
{
    // Supprime tous les clients créés
    for (AbstractClient* client : m_clients)
    {
        delete client;
    }
}


void Simulation::simulate()
{
    // La simulation avance une unité de temps à chaque tour
    for (int currentSystemTime = 0;
         currentSystemTime <= m_simulationEntry.getSimulationDuration();
         ++currentSystemTime)
    {
        m_statisticManager.simulationDurationRecord();   // Enregistre la durée

        SimulationUtility::printBankStat(currentSystemTime, m_bank);   // Affiche l'état de la banque

        updateBank(currentSystemTime);   // Met à jour les caissiers et la file


        // Vérifie si un nouveau client doit arriver
        if (currentSystemTime % m_simulationEntry.getClientArrivalInterval() == 0)
        {
            // Génère un temps de service aléatoire
            const int serviceTime = SimulationUtility::getRandomServiceTime(
                m_simulationEntry.getMinServiceTime(),
                m_simulationEntry.getMaxServiceTime());

            AbstractClient* client = nullptr;   // Aucun client pour le moment


            // Détermine si le client est VIP
            if (SimulationUtility::isPriorityClient(
                m_simulationEntry.getPriorityClientRate()))
            {
                // Crée un client VIP
                client = new VIPClient(currentSystemTime,
                    SimulationUtility::getRandomOperation(serviceTime),
                    m_simulationEntry.getClientPatienceTime());
            }
            else
            {
                // Crée un client normal
                client = new Client(currentSystemTime,
                    SimulationUtility::getRandomOperation(serviceTime),
                    m_simulationEntry.getClientPatienceTime());
            }

            m_clients.push_back(client);   // Ajoute le client à la liste


            // Cherche un caissier libre
            Cashier* cashier = m_bank.getFreeCashier();

            if (cashier == nullptr)
            {
                // Aucun caissier libre : le client attend dans la file
                SimulationUtility::printClientArrival(currentSystemTime, false);
                m_bank.getQueue().addQueueLast(client);
            }
            else
            {
                // Un caissier est libre : le client est servi directement
                SimulationUtility::printClientArrival(currentSystemTime, true);
                serveClient(currentSystemTime, cashier, client);
            }
        }
    }
}


void Simulation::updateBank(int currentSystemTime)
{
    Queue& queue = m_bank.getQueue();   // Récupère la file d'attente


    // Parcourt tous les caissiers
    for (Cashier* cashier : m_bank.getCashiers())
    {
        if (!cashier->isFree())
        {
            m_statisticManager.cashierOccupationRecord();   // Enregistre l'occupation
            cashier->work();                                // Diminue le temps de service
        }


        // Vérifie si le service est terminé
        if (cashier->serviceFinished())
        {
            AbstractClient* leavingClient = cashier->getServingClient();   // Récupère le client

            leavingClient->setDepartureTime(currentSystemTime);   // Enregistre le départ

            SimulationUtility::printClientDeparture(currentSystemTime);   // Affiche le départ

            m_statisticManager.registerServedClient(leavingClient);   // Enregistre le client servi

            cashier->setServingClient(nullptr);   // Libère le caissier


            // S'il y a des clients dans la file
            if (!queue.isEmpty())
            {
                // Prend le prochain client et le donne au caissier
                serveClient(currentSystemTime, cashier, nextWaitingClient());
            }
        }
    }


    // Diminue la patience des clients dans la file
    queue.updateClientPatience();


    // Retire les clients qui ne sont plus patients
    for (AbstractClient* client : queue.removeImpatientClients())
    {
        client->setDepartureTime(currentSystemTime);   // Enregistre le départ

        m_statisticManager.registerNonServedClient(client);   // Enregistre le client non servi

        SimulationUtility::printClientDepartureWithoutBeingServed(currentSystemTime);
    }
}


void Simulation::serveClient(
    int currentSystemTime,
    Cashier* cashier,
    AbstractClient* client)
{
    client->setServiceStartTime(currentSystemTime);   // Enregistre le début du service

    cashier->serve(client);   // Le caissier commence le service

    SimulationUtility::printServiceTimeTrace(
        currentSystemTime,
        client->getOperation()->getServiceTime());   // Affiche le temps de service
}


AbstractClient* Simulation::nextWaitingClient()
{
    Queue& queue = m_bank.getQueue();   // Récupère la file

    AbstractClient* priorityClient = queue.findPriorityClient();   // Cherche un VIP


    if (priorityClient != nullptr)
    {
        queue.removePriorityClient(priorityClient);   // Retire le VIP de la file
        return priorityClient;                       // Retourne le VIP
    }

    return queue.getQueueFirst();   // Sinon, retourne le premier client
}


std::string Simulation::simulationResults() const
{
    std::ostringstream results;

    results << std::fixed << std::setprecision(2);   // Affiche 2 chiffres après la virgule

    results << "########## Resultats de la simulation : #####################" << '\n';

    results << "Duree de la simulation : "
            << m_simulationEntry.getSimulationDuration() << '\n';

    results << "Nombre de clients servis : "
            << m_statisticManager.servedClientCount() << '\n';

    results << "Temps moyen d'attente des clients : "
            << m_statisticManager.calculateAverageClientWaitingTime() << " \n";

    results << "Temps moyen de service des clients : "
            << m_statisticManager.calculateAverageClientServiceTime() << " \n";

    results << "Taux d'occupation des caissiers : "
            << m_statisticManager.calculateAverageCashierOccupationRate(
                   m_simulationEntry.getCashierCount()) << " % \n";

    results << "Nombre de clients non servis : "
            << m_statisticManager.nonServedClientCount() << '\n';

    results << "Taux de satisfaction des clients : "
            << m_statisticManager.calculateClientSatisfactionRate() << " %";

    return results.str();   // Retourne tous les résultats
}