// tests/QueueTest.cpp
// Tests unitaires de la classe Queue (file d'attente FIFO avec priorite VIP).
//
// Comportements verifies :
//  - une file neuve est vide ;
//  - addQueueLast ajoute un client ;
//  - getQueueFirst renvoie les clients dans l'ordre d'arrivee (FIFO) ;
//  - findPriorityClient trouve le premier VIP (ou nullptr) ;
//  - removePriorityClient retire un VIP au milieu de la file ;
//  - la patience : un client non urgent part quand elle tombe a 0 ;
//  - un client avec operation urgente ne devient jamais impatient.

#include <cassert>
#include <iostream>
#include <vector>

#include "bank/Queue.h"
#include "client/AbstractClient.h"
#include "client/Client.h"
#include "client/VIPClient.h"
#include "client/Consultation.h"
#include "client/Withdraw.h"

int main()
{
    // 1) Une file neuve est vide.
    Queue queue;
    assert(queue.isEmpty());
    std::cout << "[OK] file vide au depart\n";

    // Clients de test (operation non urgente ; le client detruit son operation).
    Client c1(0, new Consultation(5), 9);
    Client c2(1, new Consultation(5), 9);

    // 2) addQueueLast : la file n'est plus vide.
    queue.addQueueLast(&c1);
    assert(!queue.isEmpty());
    queue.addQueueLast(&c2);
    std::cout << "[OK] ajout d'un client\n";

    // 3) + 4) getQueueFirst renvoie les clients dans l'ordre d'arrivee (FIFO).
    assert(queue.getQueueFirst() == &c1);
    assert(queue.getQueueFirst() == &c2);
    assert(queue.isEmpty());
    std::cout << "[OK] recuperation du premier client + FIFO\n";

    // 5) findPriorityClient : trouve le premier VIP, nullptr s'il n'y en a pas.
    Queue queue2;
    Client normal1(0, new Consultation(5), 9);
    VIPClient vip(1, new Consultation(5), 9);
    Client normal2(2, new Consultation(5), 9);

    queue2.addQueueLast(&normal1);
    assert(queue2.findPriorityClient() == nullptr); // aucun VIP pour l'instant
    queue2.addQueueLast(&vip);
    queue2.addQueueLast(&normal2);
    assert(queue2.findPriorityClient() == &vip);
    std::cout << "[OK] recherche d'un client VIP\n";

    // 6) Priorite VIP : on retire le VIP (au milieu), le FIFO reste correct pour les autres.
    queue2.removePriorityClient(&vip);
    assert(queue2.findPriorityClient() == nullptr);
    assert(queue2.getQueueFirst() == &normal1);
    assert(queue2.getQueueFirst() == &normal2);
    std::cout << "[OK] priorite d'un VIP (retrait au milieu de la file)\n";

    // 7) Patience : un client non urgent part quand sa patience atteint 0.
    Queue queue3;
    Client impatient(0, new Consultation(5), 2); // patience = 2, operation NON urgente
    queue3.addQueueLast(&impatient);

    queue3.updateClientPatience();                    // patience : 2 -> 1
    assert(queue3.removeImpatientClients().empty());  // encore patient

    queue3.updateClientPatience();                    // patience : 1 -> 0
    std::vector<AbstractClient*> partis = queue3.removeImpatientClients();
    assert(partis.size() == 1);
    assert(partis[0] == &impatient);
    assert(queue3.isEmpty());
    std::cout << "[OK] gestion de la patience (client non urgent)\n";

    // 8) Un client avec operation urgente reste, meme si sa patience est nulle.
    Queue queue4;
    Client urgent(0, new Withdraw(5), 1); // operation URGENTE
    queue4.addQueueLast(&urgent);
    queue4.updateClientPatience();                   // patience : 1 -> 0
    assert(queue4.removeImpatientClients().empty()); // urgent = toujours patient
    assert(!queue4.isEmpty());
    std::cout << "[OK] client urgent jamais impatient\n";

    std::cout << "Queue : tous les tests sont passes.\n";
    return 0;
}
