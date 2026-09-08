#ifndef WITHDRAW_H
#define WITHDRAW_H

#include "client/AbstractOperation.h"

class Withdraw : public AbstractOperation
{
    public:
        Withdraw(int serviceTime);

        bool isUrgent() const override;
        std::string toString() const override;
};

#endif // WITHDRAW_H
