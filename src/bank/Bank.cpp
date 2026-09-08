#include "bank/Bank.h"

Bank::Bank(int cashierCount)
{
    for (int count = 1; count <= cashierCount; count++)
    {
        m_cashiers.push_back(new Cashier());
    }
}

Bank::~Bank()
{
    for (Cashier* cashier : m_cashiers)
    {
        delete cashier;
    }
    m_cashiers.clear();
}

const std::vector<Cashier*>& Bank::getCashiers() const
{
    return m_cashiers;
}

Queue& Bank::getQueue()
{
    return m_queue;
}

Cashier* Bank::getFreeCashier() const
{
    for (Cashier* cashier : m_cashiers)
    {
        if (cashier->isFree())
        {
            return cashier;
        }
    }
    return nullptr;
}

std::string Bank::toString() const
{
    std::string results = m_queue.toString() + "\n";
    results += "Cashiers : ";
    for (Cashier* cashier : m_cashiers)
    {
        results += cashier->toString();
    }
    return results;
}
