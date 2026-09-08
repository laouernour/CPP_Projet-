#ifndef CLIENT_H
#define CLIENT_H

#include "client/AbstractClient.h"

class Client : public AbstractClient
{
    public:
        Client(int arrivalTime, AbstractOperation* operation, int patienceTime);

        bool isPriority() const override;
};

#endif // CLIENT_H
