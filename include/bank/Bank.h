#ifndef BANK_H
#define BANK_H

#include <string>
#include <vector>

#include "bank/Cashier.h"
#include "bank/Queue.h"

class Bank
{
    public:
        Bank(int cashierCount);       // Création de la banque avec plusieurs caissiers
        ~Bank();

        const std::vector<Cashier*>& getCashiers() const;  // Récupérer les caissiers
        Queue& getQueue();                                  // Récupérer la file d'attente

        Cashier* getFreeCashier() const;   // Chercher un caissier libre

        std::string toString() const;      // Afficher l'état de la banque

    private:
        std::vector<Cashier*> m_cashiers;  // Liste des caissiers
        Queue m_queue;                     // File d'attente commune
};

#endif // BANK_H