#include "bank/Queue.h"   // Utilise la classe Queue

bool Queue::isEmpty() const
{
    return m_clients.empty();   // Vérifie si la file est vide
}

void Queue::addQueueLast(AbstractClient* client)
{
    m_clients.push_back(client);   // Ajoute le client à la fin
}

AbstractClient* Queue::getQueueFirst()
{
    AbstractClient* first = m_clients.front();   // Récupère le premier client
    m_clients.pop_front();                      // Retire le premier client de la file
    return first;                               // Retourne ce client
}

AbstractClient* Queue::findPriorityClient() const
{
    // Cherche un client VIP dans la file
    for (AbstractClient* client : m_clients)
    {
        if (client->isPriority())   // Vérifie si le client est prioritaire
        {
            return client;          // Retourne le premier VIP trouvé
        }
    }

    return nullptr;   // Aucun VIP trouvé
}

void Queue::removePriorityClient(AbstractClient* client)
{
    m_clients.remove(client);   // Retire le client de la file
}

void Queue::updateClientPatience()
{
    // Diminue la patience de chaque client
    for (AbstractClient* client : m_clients)
    {
        client->reducePatience();
    }
}

std::vector<AbstractClient*> Queue::removeImpatientClients()
{
    std::vector<AbstractClient*> removeList;   // Liste des clients à retirer

    for (AbstractClient* client : m_clients)
    {
        if (!client->isPatient())   // Vérifie si le client n'est plus patient
        {
            removeList.push_back(client);   // Ajoute le client à la liste
        }
    }

    // Retire les clients impatients de la file
    for (AbstractClient* client : removeList)
    {
        m_clients.remove(client);
    }

    return removeList;   // Retourne les clients retirés
}

std::string Queue::toString() const
{
    std::string results = "Taille de la file[" + std::to_string(m_clients.size()) + "] : ";

    // Ajoute les clients dans l'affichage
    for (AbstractClient* client : m_clients)
    {
        results += client->toString() + "-->";
    }

    return results;   // Retourne l'état de la file
}