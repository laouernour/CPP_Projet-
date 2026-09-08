#include <iostream>
#include <vector>

#include "client/AbstractOperation.h"
#include "client/Consultation.h"
#include "client/Transfer.h"
#include "client/Withdraw.h"
#include "client/AbstractClient.h"
#include "client/Client.h"
#include "client/VIPClient.h"
#include "bank/Cashier.h"
#include "bank/Queue.h"
#include "bank/Bank.h"

using namespace std;

/**
 * Test de fumee : verifie que toutes les classes des paquets client/ et bank/
 * compilent et fonctionnent ensemble (polymorphisme, file d'attente, caissiers).
 */
int main()
{
    // --- Operations (polymorphisme sur AbstractOperation) ---
    cout << "=== Operations ===" << endl;
    vector<AbstractOperation*> operations;
    operations.push_back(new Consultation(12));
    operations.push_back(new Transfer(8));
    operations.push_back(new Withdraw(15));
    for (AbstractOperation* op : operations)
    {
        cout << op->toString()
             << " | serviceTime = " << op->getServiceTime()
             << " | urgent = " << (op->isUrgent() ? "oui" : "non") << endl;
        delete op;
    }

    // --- Banque + file d'attente ---
    cout << "\n=== Banque (2 caissiers) ===" << endl;
    Bank bank(2);

    AbstractClient* c1 = new Client(0, new Consultation(3), 5);
    AbstractClient* c2 = new VIPClient(1, new Withdraw(4), 5);
    AbstractClient* c3 = new Client(2, new Transfer(2), 5);

    bank.getQueue().addQueueLast(c1);
    bank.getQueue().addQueueLast(c2);
    bank.getQueue().addQueueLast(c3);
    cout << bank.toString() << endl;

    // Le caissier libre prend en priorite le client VIP
    Cashier* free = bank.getFreeCashier();
    AbstractClient* priority = bank.getQueue().findPriorityClient();
    if (priority != nullptr)
    {
        bank.getQueue().removePriorityClient(priority);
        free->serve(priority);
        cout << "\nLe caissier sert en priorite : " << priority->toString() << endl;
    }
    cout << bank.toString() << endl;

    // Simule quelques unites de temps
    cout << "\n=== 5 unites de temps ===" << endl;
    for (int t = 0; t < 5; t++)
    {
        for (Cashier* cashier : bank.getCashiers())
        {
            cashier->work();
        }
        bank.getQueue().updateClientPatience();
        cout << "t=" << t << " -> " << bank.toString() << endl;
    }

    // Nettoyage
    delete c1;
    delete c2;
    delete c3;

    cout << "\nOK : bank2_cpp (client/ + bank/) compile et s'execute." << endl;
    return 0;
}
