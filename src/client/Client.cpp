#include "client/Client.h"

Client::Client(int arrivalTime, AbstractOperation* operation, int patienceTime)
    : AbstractClient(arrivalTime, operation, patienceTime)
{
}

bool Client::isPriority() const
{
    return false;
}
