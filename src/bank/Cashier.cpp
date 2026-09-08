#include "bank/Cashier.h"

Cashier::Cashier() : m_servingClient(nullptr), m_remainingServiceTime(0)
{
}

bool Cashier::isFree() const
{
    return m_servingClient == nullptr;
}

void Cashier::work()
{
    if (m_remainingServiceTime > 0)
    {
        m_remainingServiceTime--;
    }
}

bool Cashier::serviceFinished() const
{
    return m_servingClient != nullptr && m_remainingServiceTime == 0;
}

void Cashier::serve(AbstractClient* servingClient)
{
    m_servingClient = servingClient;
    m_remainingServiceTime = servingClient->getOperation()->getServiceTime();
}

AbstractClient* Cashier::getServingClient() const
{
    return m_servingClient;
}

void Cashier::setServingClient(AbstractClient* servingClient)
{
    m_servingClient = servingClient;
}

std::string Cashier::toString() const
{
    return "Caissier[Temps restant : " + std::to_string(m_remainingServiceTime) + "]";
}
