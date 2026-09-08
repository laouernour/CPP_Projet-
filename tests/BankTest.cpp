// tests/BankTest.cpp
// Tests unitaires de la classe Bank (caissiers + file d'attente partagee).
//
// Comportements verifies :
//  - Bank(n) cree exactement n caissiers (tous non nuls) ;
//  - getCashiers() : au depart tous les caissiers sont libres ;
//  - getQueue() renvoie la file partagee, vide au depart ;
//  - getFreeCashier() renvoie un caissier libre tant qu'il en reste, sinon nullptr,
//    et retrouve un caissier qu'on vient de liberer.

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

#include "bank/Bank.h"
#include "bank/Cashier.h"
#include "bank/Queue.h"
#include "client/Client.h"
#include "client/Consultation.h"

int main()
{
    // 1) Bank(3) cree 3 caissiers, tous non nuls.
    Bank bank(3);
    assert(bank.getCashiers().size() == 3);
    for (Cashier* cashier : bank.getCashiers())
    {
        assert(cashier != nullptr);
    }
    std::cout << "[OK] creation du bon nombre de caissiers\n";

    // 2) getCashiers() : au depart, tous les caissiers sont libres.
    for (Cashier* cashier : bank.getCashiers())
    {
        assert(cashier->isFree());
    }
    std::cout << "[OK] recuperation des caissiers (tous libres)\n";

    // 3) getQueue() renvoie la file partagee, vide au depart.
    assert(bank.getQueue().isEmpty());
    std::cout << "[OK] recuperation de la file\n";

    // 4) getFreeCashier() : un caissier libre existe au depart.
    assert(bank.getFreeCashier() != nullptr);

    // On occupe les 3 caissiers avec 3 clients (crees sur le tas : liberes en fin de test).
    std::vector<Client*> clients;
    for (int i = 0; i < 3; ++i)
    {
        clients.push_back(new Client(i, new Consultation(5), 9));
    }
    const std::vector<Cashier*>& cashiers = bank.getCashiers();
    for (std::size_t i = 0; i < cashiers.size(); ++i)
    {
        cashiers[i]->serve(clients[i]);
    }
    assert(bank.getFreeCashier() == nullptr); // plus aucun caissier libre
    std::cout << "[OK] getFreeCashier() == nullptr quand tout est occupe\n";

    // On libere le premier caissier : c'est lui que getFreeCashier() retrouve.
    cashiers[0]->setServingClient(nullptr);
    assert(bank.getFreeCashier() == cashiers[0]);
    std::cout << "[OK] getFreeCashier() retrouve un caissier libere\n";

    // Menage : on detache les clients des caissiers, puis on les detruit.
    for (Cashier* cashier : cashiers)
    {
        cashier->setServingClient(nullptr);
    }
    for (Client* client : clients)
    {
        delete client;
    }

    std::cout << "Bank : tous les tests sont passes.\n";
    return 0;
}
