#include "client/AbstractClient.h"   // Utilise la classe AbstractClient

AbstractClient::AbstractClient(int arrivalTime, AbstractOperation* operation, int patienceTime)
    : m_arrivalTime(arrivalTime),          // Heure d'arrivée du client
      m_serviceStartTime(0),               // Service pas encore commencé
      m_departureTime(0),                  // Client pas encore parti
      m_operation(operation),              // Opération du client
      m_patienceTime(patienceTime)          // Temps de patience
{
}

AbstractClient::~AbstractClient()
{
    delete m_operation;   // Supprime l'opération
}

int AbstractClient::getArrivalTime() const
{
    return m_arrivalTime;   // Retourne l'heure d'arrivée
}

int AbstractClient::getDepartureTime() const
{
    return m_departureTime;   // Retourne l'heure de départ
}

void AbstractClient::setDepartureTime(int departureTime)
{
    m_departureTime = departureTime;   // Définit l'heure de départ
}

int AbstractClient::getServiceStartTime() const
{
    return m_serviceStartTime;   // Retourne le début du service
}

void AbstractClient::setServiceStartTime(int serviceStartTime)
{
    m_serviceStartTime = serviceStartTime;   // Définit le début du service
}

AbstractOperation* AbstractClient::getOperation() const
{
    return m_operation;   // Retourne l'opération
}

void AbstractClient::reducePatience()
{
    if (m_patienceTime > 0)   // Vérifie s'il reste de la patience
    {
        m_patienceTime--;     // Diminue la patience de 1
    }
}

bool AbstractClient::isPatient() const
{
    return m_patienceTime > 0 || m_operation->isUrgent();
    // Patient s'il a encore de la patience ou si l'opération est urgente
}

std::string AbstractClient::toString() const
{
    return "Client[arrivee : " + std::to_string(m_arrivalTime)
         + ", temps de patience : " + std::to_string(m_patienceTime)
         + ", " + m_operation->toString() + "]";
    // Retourne les informations du client
}