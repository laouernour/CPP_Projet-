#ifndef TRANSFER_H
#define TRANSFER_H

#include "client/AbstractOperation.h"   // On hérite de AbstractOperation

class Transfer : public AbstractOperation
{
    public:
        Transfer(int serviceTime);
        // Crée une opération de transfert avec son temps de service

        bool isUrgent() const override;
        // Indique que le transfert est urgent

        std::string toString() const override;
        // Retourne les informations de l'opération
};

#endif // TRANSFER_H