#ifndef WITHDRAW_H
#define WITHDRAW_H

#include "client/AbstractOperation.h"   // On hérite de AbstractOperation

class Withdraw : public AbstractOperation
{
    public:
        Withdraw(int serviceTime);
        // Crée une opération de retrait avec son temps de service

        bool isUrgent() const override;
        // Indique que le retrait est urgent

        std::string toString() const override;
        // Retourne les informations de l'opération
};

#endif // WITHDRAW_H