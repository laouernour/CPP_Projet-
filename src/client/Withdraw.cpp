#include "client/Withdraw.h"

Withdraw::Withdraw(int serviceTime) : AbstractOperation(serviceTime)
{
}

bool Withdraw::isUrgent() const
{
    return true;
}

std::string Withdraw::toString() const
{
    return "Operation : Retrait";
}
