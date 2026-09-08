#ifndef CLIENT_H
#define CLIENT_H

#include "client/AbstractClient.h"   // On hérite de AbstractClient

class Client : public AbstractClient
{
    public:
        Client(int arrivalTime, AbstractOperation* operation, int patienceTime);
        // Crée un client avec son arrivée, son opération et sa patience

        bool isPriority() const override;   // Indique que ce client n'est pas VIP
};

#endif // CLIENT_H