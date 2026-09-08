#include "client/VIPClient.h"   // Utilise la classe VIPClient

VIPClient::VIPClient(int arrivalTime, AbstractOperation* operation, int patienceTime)
    : AbstractClient(arrivalTime, operation, patienceTime)   // Initialise le client
{
}

bool VIPClient::isPriority() const
{
    return true;   // Le client VIP est prioritaire
}

std::string VIPClient::toString() const
{
    return "Prioritaire " + AbstractClient::toString();
    // Ajoute "Prioritaire" aux informations du client
}