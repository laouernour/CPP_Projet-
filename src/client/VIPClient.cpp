#include "client/VIPClient.h"

VIPClient::VIPClient(int arrivalTime, AbstractOperation* operation, int patienceTime)
    : AbstractClient(arrivalTime, operation, patienceTime)
{
}

bool VIPClient::isPriority() const
{
    return true;
}

std::string VIPClient::toString() const
{
    return "Priority " + AbstractClient::toString();
}
