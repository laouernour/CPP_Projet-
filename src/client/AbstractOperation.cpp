#include "client/AbstractOperation.h"   // Utilise la classe AbstractOperation

AbstractOperation::AbstractOperation(int serviceTime)
    : m_serviceTime(serviceTime)        // Enregistre le temps de service
{
}

AbstractOperation::~AbstractOperation()
{
    // Rien à supprimer ici
}

int AbstractOperation::getServiceTime() const
{
    return m_serviceTime;   // Retourne le temps de service
}