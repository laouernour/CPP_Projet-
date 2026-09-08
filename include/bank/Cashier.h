#ifndef CASHIER_H
#define CASHIER_H

#include <string>                   

#include "client/AbstractClient.h"   // On utilise la classe AbstractClient

class Cashier
{
    public:
        Cashier();   // Crée un caissier

        bool isFree() const;   // Vérifie si le caissier est libre

        void work();   // Fait avancer le temps de service

        bool serviceFinished() const;   // Vérifie si le service est terminé

        void serve(AbstractClient* servingClient);   // Commence à servir un client

        AbstractClient* getServingClient() const;   // Récupère le client servi

        void setServingClient(AbstractClient* servingClient);   // Change le client servi

        std::string toString() const;   // Retourne l'état du caissier

    private:
        AbstractClient* m_servingClient;   // Client actuellement servi
        int m_remainingServiceTime;        // Temps de service restant
};

#endif // CASHIER_H