#ifndef QUEUE_H
#define QUEUE_H

#include <list>                       // Permet de stocker les clients
#include <string>                     // Permet d'utiliser std::string
#include <vector>                     // Permet de retourner une liste de clients

#include "client/AbstractClient.h"     // On utilise AbstractClient

class Queue
{
    public:
        bool isEmpty() const;   // Vérifie si la file est vide

        void addQueueLast(AbstractClient* client);   // Ajoute un client à la fin

        AbstractClient* getQueueFirst();   // Récupère le premier client

        AbstractClient* findPriorityClient() const;   // Cherche le premier client VIP

        void removePriorityClient(AbstractClient* client);   // Retire un client VIP

        void updateClientPatience();   // Diminue la patience des clients

        std::vector<AbstractClient*> removeImpatientClients();   // Retire les clients impatients

        std::string toString() const;   // Retourne l'état de la file

        int size() const;   // Nombre de clients en attente

        const std::list<AbstractClient*>& items() const;   // Accès en lecture (affichage)

    private:
        std::list<AbstractClient*> m_clients;   // Liste des clients en attente
};

#endif // QUEUE_H