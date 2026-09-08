#include "bank/Bank.h"   // Utilise la classe Bank

Bank::Bank(int cashierCount)
{
    // Crée le nombre de caissiers demandé
    for (int count = 1; count <= cashierCount; count++)
    {
        m_cashiers.push_back(new Cashier());   // Ajoute un caissier
    }
}

Bank::~Bank()
{
    // Supprime tous les caissiers
    for (Cashier* cashier : m_cashiers)
    {
        delete cashier;   // Libère la mémoire du caissier
    }

    m_cashiers.clear();   // Vide la liste
}

const std::vector<Cashier*>& Bank::getCashiers() const
{
    return m_cashiers;   // Retourne les caissiers
}

Queue& Bank::getQueue()
{
    return m_queue;   // Retourne la file d'attente
}

Cashier* Bank::getFreeCashier() const
{
    // Cherche un caissier libre
    for (Cashier* cashier : m_cashiers)
    {
        if (cashier->isFree())   // Vérifie s'il est libre
        {
            return cashier;   // Retourne le caissier libre
        }
    }

    return nullptr;   // Aucun caissier libre
}

std::string Bank::toString() const
{
    std::string results = m_queue.toString() + "\n";   // État de la file

    results += "Caissiers : ";

    for (Cashier* cashier : m_cashiers)
    {
        results += cashier->toString();   // Ajoute l'état du caissier
    }

    return results;   // Retourne l'état de la banque
}