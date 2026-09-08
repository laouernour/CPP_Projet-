#include "client/Withdraw.h"   // Utilise la classe Withdraw

Withdraw::Withdraw(int serviceTime)
    : AbstractOperation(serviceTime)   // Initialise le temps de service
{
}

bool Withdraw::isUrgent() const
{
    return true;   // Un retrait est urgent
}

std::string Withdraw::toString() const
{
    return "Operation : Retrait";   // Retourne le nom de l'opération
}