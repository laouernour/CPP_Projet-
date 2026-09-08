#ifndef QUEUE_H
#define QUEUE_H

#include <list>
#include <string>
#include <vector>

#include "client/AbstractClient.h"

/**
 * File d'attente FIFO de clients. Les pointeurs sont des references : la file ne
 * detruit pas les clients.
 */
class Queue
{
    public:
        bool isEmpty() const;

        void addQueueLast(AbstractClient* client);

        /** Retire et retourne le premier client de la file. */
        AbstractClient* getQueueFirst();

        /** @return le client prioritaire le plus proche de la tete, ou nullptr. */
        AbstractClient* findPriorityClient() const;

        void removePriorityClient(AbstractClient* client);

        void updateClientPatience();

        /** Retire de la file et retourne la liste des clients devenus impatients. */
        std::vector<AbstractClient*> removeImpatientClients();

        std::string toString() const;

    private:
        std::list<AbstractClient*> m_clients;
};

#endif // QUEUE_H
