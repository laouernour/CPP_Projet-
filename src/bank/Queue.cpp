#include "bank/Queue.h"

bool Queue::isEmpty() const
{
    return m_clients.empty();
}

void Queue::addQueueLast(AbstractClient* client)
{
    m_clients.push_back(client);
}

AbstractClient* Queue::getQueueFirst()
{
    AbstractClient* first = m_clients.front();
    m_clients.pop_front();
    return first;
}

AbstractClient* Queue::findPriorityClient() const
{
    for (AbstractClient* client : m_clients)
    {
        if (client->isPriority())
        {
            return client;
        }
    }
    return nullptr;
}

void Queue::removePriorityClient(AbstractClient* client)
{
    m_clients.remove(client);
}

void Queue::updateClientPatience()
{
    for (AbstractClient* client : m_clients)
    {
        client->reducePatience();
    }
}

std::vector<AbstractClient*> Queue::removeImpatientClients()
{
    std::vector<AbstractClient*> removeList;
    for (AbstractClient* client : m_clients)
    {
        if (!client->isPatient())
        {
            removeList.push_back(client);
        }
    }

    for (AbstractClient* client : removeList)
    {
        m_clients.remove(client);
    }
    return removeList;
}

std::string Queue::toString() const
{
    std::string results = "Queue size[" + std::to_string(m_clients.size()) + "] : ";
    for (AbstractClient* client : m_clients)
    {
        results += client->toString() + "-->";
    }
    return results;
}
