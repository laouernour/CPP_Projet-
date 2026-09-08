#include "client/Transfer.h"   // Utilise la classe Transfer

Transfer::Transfer(int serviceTime)
    : AbstractOperation(serviceTime)   // Initialise le temps de service
{
}

bool Transfer::isUrgent() const
{
    return true;   // Un virement est urgent
}

std::string Transfer::toString() const
{
    return "Operation : Virement";   // Retourne le nom de l'opération
}