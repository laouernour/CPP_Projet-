#ifndef VIPCLIENT_H
#define VIPCLIENT_H

#include "client/AbstractClient.h"   // On hérite de AbstractClient

class VIPClient : public AbstractClient
{
    public:
        VIPClient(int arrivalTime, AbstractOperation* operation, int patienceTime);
        // Crée un client VIP

        bool isPriority() const override;   // Indique que le client est prioritaire

        std::string toString() const override;   // Retourne les informations du VIP
};

#endif // VIPCLIENT_H