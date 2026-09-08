#include "bank/Cashier.h"   // Utilise la classe Cashier

Cashier::Cashier() : m_servingClient(nullptr), m_remainingServiceTime(0)
{
    // Au début, le caissier est libre et n'a aucun temps de service
}

bool Cashier::isFree() const
{
    return m_servingClient == nullptr;   // Vrai si aucun client n'est servi
}

void Cashier::work()
{
    if (m_remainingServiceTime > 0)   // Vérifie s'il reste du temps
    {
        m_remainingServiceTime--;     // Diminue le temps de 1
    }
}

bool Cashier::serviceFinished() const
{
    return m_servingClient != nullptr && m_remainingServiceTime == 0;
    // Vrai si un client est présent et que son service est terminé
}

void Cashier::serve(AbstractClient* servingClient)
{
    m_servingClient = servingClient;   // Enregistre le client

    m_remainingServiceTime = servingClient->getOperation()->getServiceTime();
    // Récupère le temps de service de l'opération
}

AbstractClient* Cashier::getServingClient() const
{
    return m_servingClient;   // Retourne le client servi
}

void Cashier::setServingClient(AbstractClient* servingClient)
{
    m_servingClient = servingClient;   // Change le client servi
}

std::string Cashier::toString() const
{
    return "Caissier[Temps restant : " + std::to_string(m_remainingServiceTime) + "]";
    // Retourne le temps de service restant
}