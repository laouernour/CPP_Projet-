// tests/CashierTest.cpp
// Tests unitaires de la classe Cashier (un guichet qui sert un client a la fois).
//
// Comportements verifies :
//  - un caissier neuf est libre ;
//  - serve() rend le caissier occupe et memorise le client ;
//  - work() fait diminuer le temps de service restant ;
//  - serviceFinished() detecte la fin du service ;
//  - liberer le caissier le rend a nouveau libre.

#include <cassert>
#include <iostream>

#include "bank/Cashier.h"
#include "client/Client.h"
#include "client/Consultation.h"

int main()
{
    // 1) Un caissier neuf est libre et ne sert personne.
    Cashier cashier;
    assert(cashier.isFree());
    assert(cashier.getServingClient() == nullptr);
    assert(!cashier.serviceFinished());
    std::cout << "[OK] caissier neuf = libre\n";

    // Un client dont l'operation dure 3 unites de temps.
    // Le client est proprietaire de son operation : il la detruira lui-meme.
    Client client(0, new Consultation(3), 5);

    // 2) serve() : le caissier devient occupe et retient le client servi.
    cashier.serve(&client);
    assert(!cashier.isFree());
    assert(cashier.getServingClient() == &client);
    assert(!cashier.serviceFinished()); // il reste encore 3 unites
    std::cout << "[OK] serve() -> caissier occupe\n";

    // 3) work() decremente le temps de service restant.
    cashier.work(); // 3 -> 2
    assert(!cashier.serviceFinished());
    cashier.work(); // 2 -> 1
    assert(!cashier.serviceFinished());

    // 4) Apres la derniere unite de travail, le service est detecte comme termine.
    cashier.work(); // 1 -> 0
    assert(cashier.serviceFinished());
    std::cout << "[OK] work() puis serviceFinished()\n";

    // 5) Liberer le caissier : il redevient libre et n'est plus "termine".
    cashier.setServingClient(nullptr);
    assert(cashier.isFree());
    assert(!cashier.serviceFinished());
    std::cout << "[OK] caissier libere\n";

    std::cout << "Cashier : tous les tests sont passes.\n";
    return 0;
}
