#include "client/AbstractOperation.h"

AbstractOperation::AbstractOperation(int serviceTime) : m_serviceTime(serviceTime)
{
}

AbstractOperation::~AbstractOperation()
{
}

int AbstractOperation::getServiceTime() const
{
    return m_serviceTime;
}
