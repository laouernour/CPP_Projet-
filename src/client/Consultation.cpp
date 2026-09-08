#include "client/Consultation.h"

Consultation::Consultation(int serviceTime) : AbstractOperation(serviceTime)
{
}

bool Consultation::isUrgent() const
{
    return false;
}

std::string Consultation::toString() const
{
    return "Operation : Consultation";
}
