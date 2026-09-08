#include "simulation/Simulation.h"

#include <iomanip>
#include <sstream>

#include "bank/Cashier.h"
#include "bank/Queue.h"
#include "client/AbstractClient.h"
#include "client/Client.h"
#include "client/VIPClient.h"
#include "simulation/SimulationUtility.h"

Simulation::Simulation(const SimulationEntry& simulationEntry)
    : m_simulationEntry(simulationEntry), m_bank(simulationEntry.getCashierCount())
{
}

Simulation::~Simulation()
{
    for (AbstractClient* client : m_clients)
    {
        delete client;
    }
}

void Simulation::simulate()
{
    for (int currentSystemTime = 0;
         currentSystemTime <= m_simulationEntry.getSimulationDuration();
         ++currentSystemTime)
    {
        m_statisticManager.simulationDurationRecord();
        SimulationUtility::printBankStat(currentSystemTime, m_bank);
        updateBank(currentSystemTime);

        if (currentSystemTime % m_simulationEntry.getClientArrivalInterval() == 0)
        {
            const int serviceTime = SimulationUtility::getRandomServiceTime(
                m_simulationEntry.getMinServiceTime(), m_simulationEntry.getMaxServiceTime());
            AbstractClient* client = nullptr;
            if (SimulationUtility::isPriorityClient(m_simulationEntry.getPriorityClientRate()))
            {
                client = new VIPClient(currentSystemTime,
                    SimulationUtility::getRandomOperation(serviceTime),
                    m_simulationEntry.getClientPatienceTime());
            }
            else
            {
                client = new Client(currentSystemTime,
                    SimulationUtility::getRandomOperation(serviceTime),
                    m_simulationEntry.getClientPatienceTime());
            }
            m_clients.push_back(client);

            Cashier* cashier = m_bank.getFreeCashier();
            if (cashier == nullptr)
            {
                SimulationUtility::printClientArrival(currentSystemTime, false);
                m_bank.getQueue().addQueueLast(client);
            }
            else
            {
                SimulationUtility::printClientArrival(currentSystemTime, true);
                serveClient(currentSystemTime, cashier, client);
            }
        }
    }
}

void Simulation::updateBank(int currentSystemTime)
{
    Queue& queue = m_bank.getQueue();
    for (Cashier* cashier : m_bank.getCashiers())
    {
        if (!cashier->isFree())
        {
            m_statisticManager.cashierOccupationRecord();
            cashier->work();
        }

        if (cashier->serviceFinished())
        {
            AbstractClient* leavingClient = cashier->getServingClient();
            leavingClient->setDepartureTime(currentSystemTime);
            SimulationUtility::printClientDeparture(currentSystemTime);
            m_statisticManager.registerServedClient(leavingClient);
            cashier->setServingClient(nullptr);

            if (!queue.isEmpty())
            {
                serveClient(currentSystemTime, cashier, nextWaitingClient());
            }
        }
    }

    queue.updateClientPatience();
    for (AbstractClient* client : queue.removeImpatientClients())
    {
        client->setDepartureTime(currentSystemTime);
        m_statisticManager.registerNonServedClient(client);
        SimulationUtility::printClientDepartureWithoutBeingServed(currentSystemTime);
    }
}

void Simulation::serveClient(int currentSystemTime, Cashier* cashier, AbstractClient* client)
{
    client->setServiceStartTime(currentSystemTime);
    cashier->serve(client);
    SimulationUtility::printServiceTimeTrace(currentSystemTime,
        client->getOperation()->getServiceTime());
}

AbstractClient* Simulation::nextWaitingClient()
{
    Queue& queue = m_bank.getQueue();
    AbstractClient* priorityClient = queue.findPriorityClient();
    if (priorityClient != nullptr)
    {
        queue.removePriorityClient(priorityClient);
        return priorityClient;
    }
    return queue.getQueueFirst();
}

std::string Simulation::simulationResults() const
{
    std::ostringstream results;
    results << std::fixed << std::setprecision(2);
    results << "########## Resultats de la simulation : #####################" << '\n';
    results << "Duree de la simulation : " << m_simulationEntry.getSimulationDuration() << '\n';
    results << "Nombre de clients servis : " << m_statisticManager.servedClientCount() << '\n';
    results << "Temps moyen d'attente des clients : " << m_statisticManager.calculateAverageClientWaitingTime() << " \n";
    results << "Temps moyen de service des clients : " << m_statisticManager.calculateAverageClientServiceTime() << " \n";
    results << "Taux d'occupation des caissiers : "
            << m_statisticManager.calculateAverageCashierOccupationRate(
                   m_simulationEntry.getCashierCount()) << " % \n";
    results << "Nombre de clients non servis : " << m_statisticManager.nonServedClientCount() << '\n';
    results << "Taux de satisfaction des clients : "
            << m_statisticManager.calculateClientSatisfactionRate() << " %";
    return results.str();
}
