#include "client/AbstractClient.h"

AbstractClient::AbstractClient(int arrivalTime, AbstractOperation* operation, int patienceTime)
    : m_arrivalTime(arrivalTime),
      m_serviceStartTime(0),
      m_departureTime(0),
      m_operation(operation),
      m_patienceTime(patienceTime)
{
}

AbstractClient::~AbstractClient()
{
    delete m_operation;
}

int AbstractClient::getArrivalTime() const
{
    return m_arrivalTime;
}

int AbstractClient::getDepartureTime() const
{
    return m_departureTime;
}

void AbstractClient::setDepartureTime(int departureTime)
{
    m_departureTime = departureTime;
}

int AbstractClient::getServiceStartTime() const
{
    return m_serviceStartTime;
}

void AbstractClient::setServiceStartTime(int serviceStartTime)
{
    m_serviceStartTime = serviceStartTime;
}

AbstractOperation* AbstractClient::getOperation() const
{
    return m_operation;
}

void AbstractClient::reducePatience()
{
    if (m_patienceTime > 0)
    {
        m_patienceTime--;
    }
}

bool AbstractClient::isPatient() const
{
    return m_patienceTime > 0 || m_operation->isUrgent();
}

std::string AbstractClient::toString() const
{
    return "Client[arrivee : " + std::to_string(m_arrivalTime)
         + ", temps de patience : " + std::to_string(m_patienceTime)
         + ", " + m_operation->toString() + "]";
}
