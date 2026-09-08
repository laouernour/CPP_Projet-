// tests/ClientTest.cpp
// Tests unitaires des classes Client et VIPClient (heritage + polymorphisme).
//
// Comportements verifies :
//  - un Client normal n'est pas prioritaire ;
//  - un VIPClient est prioritaire ;
//  - l'appel via un pointeur AbstractClient* choisit la bonne methode (polymorphisme) ;
//  - les getters herites renvoient les donnees passees au constructeur ;
//  - isPatient() suit la valeur de la patience ;
//  - VIPClient redefinit toString().

#include <cassert>
#include <iostream>
#include <string>

#include "client/AbstractClient.h"
#include "client/Client.h"
#include "client/VIPClient.h"
#include "client/Consultation.h"

int main()
{
    Client    client(4, new Consultation(12), 3);
    VIPClient vipClient(4, new Consultation(12), 3);

    // 1) Un Client normal n'est pas prioritaire.
    assert(client.isPriority() == false);
    std::cout << "[OK] Client normal non prioritaire\n";

    // 2) Un VIPClient est prioritaire.
    assert(vipClient.isPriority() == true);
    std::cout << "[OK] VIPClient prioritaire\n";

    // 3) Polymorphisme : via un pointeur AbstractClient*, la bonne methode est appelee.
    AbstractClient* base1 = &client;
    AbstractClient* base2 = &vipClient;
    assert(base1->isPriority() == false);
    assert(base2->isPriority() == true);
    std::cout << "[OK] polymorphisme sur isPriority()\n";

    // 4) Les getters herites renvoient les donnees du constructeur.
    assert(base1->getArrivalTime() == 4);
    assert(base1->getOperation() != nullptr);
    assert(base1->getOperation()->getServiceTime() == 12);
    std::cout << "[OK] getters herites (arrivee, operation)\n";

    // 5) isPatient() : vrai tant que la patience > 0 (operation non urgente), faux ensuite.
    assert(client.isPatient());
    client.reducePatience();
    client.reducePatience();
    client.reducePatience(); // patience : 3 -> 0
    assert(!client.isPatient());
    std::cout << "[OK] isPatient() suit la patience\n";

    // 6) VIPClient redefinit toString() : le texte commence par "Prioritaire".
    assert(vipClient.toString().rfind("Prioritaire", 0) == 0);
    std::cout << "[OK] VIPClient::toString() redefini\n";

    std::cout << "Client / VIPClient : tous les tests sont passes.\n";
    return 0;
}
