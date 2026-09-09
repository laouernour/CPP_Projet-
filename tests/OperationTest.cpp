// tests/OperationTest.cpp
// Tests unitaires des operations : Consultation, Transfer, Withdraw.
//
// Comportements verifies :
//  - Consultation est NON urgente ;
//  - Transfer est urgente ;
//  - Withdraw est urgente ;
//  - getServiceTime() renvoie la valeur passee au constructeur (attribut de la classe mere) ;
//  - toString() renvoie un libelle propre a chaque type ;
//  - via un pointeur AbstractOperation*, isUrgent() reste correct (polymorphisme).

#include <cassert>
#include <iostream>
#include <string>

#include "client/AbstractOperation.h"
#include "client/Consultation.h"
#include "client/Transfer.h"
#include "client/Withdraw.h"

int main()
{
    Consultation consultation(10);
    Transfer     transfer(15);
    Withdraw     withdraw(20);

    // 1) Consultation : non urgente.
    assert(consultation.isUrgent() == false);
    std::cout << "[OK] Consultation non urgente\n";

    // 2) Transfer : urgente.
    assert(transfer.isUrgent() == true);
    std::cout << "[OK] Transfer urgente\n";

    // 3) Withdraw : urgente.
    assert(withdraw.isUrgent() == true);
    std::cout << "[OK] Withdraw urgente\n";

    // 4) getServiceTime() renvoie la valeur passee au constructeur.
    assert(consultation.getServiceTime() == 10);
    assert(transfer.getServiceTime() == 15);
    assert(withdraw.getServiceTime() == 20);
    std::cout << "[OK] temps de service conserve\n";

    // 5) toString() : un libelle distinct par type d'operation.
    assert(consultation.toString() == "Operation : Consultation");
    assert(transfer.toString() == "Operation : Virement");
    assert(withdraw.toString() == "Operation : Retrait");
    std::cout << "[OK] toString() par type\n";

    // 6) typeName() : nom court utilise pour la base de donnees.
    assert(consultation.typeName() == "consultation");
    assert(transfer.typeName() == "transfer");
    assert(withdraw.typeName() == "withdraw");
    std::cout << "[OK] typeName() par type\n";

    // 7) Polymorphisme : via un pointeur de base, isUrgent() appelle la bonne version.
    AbstractOperation* op = &transfer;
    assert(op->isUrgent() == true);
    assert(op->getServiceTime() == 15);
    assert(op->typeName() == "transfer");
    std::cout << "[OK] polymorphisme sur isUrgent()\n";

    std::cout << "Operations : tous les tests sont passes.\n";
    return 0;
}
