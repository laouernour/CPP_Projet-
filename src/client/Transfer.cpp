#include "client/Transfer.h"

Transfer::Transfer(int serviceTime) : AbstractOperation(serviceTime)
{
}

bool Transfer::isUrgent() const
{
    return true;
}

std::string Transfer::toString() const
{
    return "Operation : Virement";
}
