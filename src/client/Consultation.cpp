#include "client/Consultation.h"   // Utilise la classe Consultation

Consultation::Consultation(int serviceTime)
    : AbstractOperation(serviceTime)   // Initialise le temps de service
{
}

bool Consultation::isUrgent() const
{
    return false;   // Une consultation n'est pas urgente
}

std::string Consultation::toString() const
{
    return "Operation : Consultation";   // Retourne le nom de l'opération
}

std::string Consultation::typeName() const
{
    return "consultation";   // Nom court utilisé pour la base de données
}