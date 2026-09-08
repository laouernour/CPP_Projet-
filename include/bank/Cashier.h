#ifndef CASHIER_H
#define CASHIER_H

#include <string>

#include "client/AbstractClient.h"

/**
 * Un caissier sert au plus un client a la fois. Le pointeur servingClient est
 * une reference (le caissier ne detruit pas le client).
 */
class Cashier
{
    public:
        Cashier();

        bool isFree() const;

        /** Fait travailler le caissier pendant une unite de temps. */
        void work();

        /** @return true si le caissier vient de terminer le service courant. */
        bool serviceFinished() const;

        /** Commence le service d'un nouveau client. */
        void serve(AbstractClient* servingClient);

        AbstractClient* getServingClient() const;
        void setServingClient(AbstractClient* servingClient);

        std::string toString() const;

    private:
        AbstractClient* m_servingClient;
        int m_remainingServiceTime;
};

#endif // CASHIER_H
