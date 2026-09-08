#ifndef BANK_H
#define BANK_H

#include <string>
#include <vector>

#include "bank/Cashier.h"
#include "bank/Queue.h"

/**
 * Une banque est composee de caissiers et d'une file d'attente partagee.
 * La banque est proprietaire de ses caissiers.
 */
class Bank
{
    public:
        Bank(int cashierCount);
        ~Bank();

        const std::vector<Cashier*>& getCashiers() const;
        Queue& getQueue();

        /** @return le premier caissier libre trouve, ou nullptr. */
        Cashier* getFreeCashier() const;

        std::string toString() const;

    private:
        std::vector<Cashier*> m_cashiers;
        Queue m_queue;
};

#endif // BANK_H
