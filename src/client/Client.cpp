#include "client/Client.h"   // Utilise la classe Client

Client::Client(int arrivalTime, AbstractOperation* operation, int patienceTime)
    : AbstractClient(arrivalTime, operation, patienceTime)   // Initialise le client
{
}

bool Client::isPriority() const
{
    return false;   // Un client normal n'est pas prioritaire
}