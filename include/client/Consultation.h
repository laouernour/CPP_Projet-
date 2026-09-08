#ifndef CONSULTATION_H
#define CONSULTATION_H

#include "client/AbstractOperation.h"   // On hérite de AbstractOperation

class Consultation : public AbstractOperation
{
    public:
        Consultation(int serviceTime);
        // Crée une consultation avec son temps de service

        bool isUrgent() const override;
        // Indique si la consultation est urgente

        std::string toString() const override;
        // Retourne les informations de l'opération

};

#endif // CONSULTATION_H