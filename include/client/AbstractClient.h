#ifndef ABSTRACTCLIENT_H
#define ABSTRACTCLIENT_H

#include <string>                       

#include "client/AbstractOperation.h"   // Un client possède une opération

class AbstractClient
{
    public:
        AbstractClient(int arrivalTime, AbstractOperation* operation, int patienceTime);
        virtual ~AbstractClient();   // Destructeur

        virtual bool isPriority() const = 0;   // Dit si le client est VIP

        int getArrivalTime() const;   // Récupère l'heure d'arrivée

        int getDepartureTime() const;   // Récupère l'heure de départ
        void setDepartureTime(int departureTime);   // Définit l'heure de départ

        int getServiceStartTime() const;   // Récupère le début du service
        void setServiceStartTime(int serviceStartTime);   // Définit le début du service

        AbstractOperation* getOperation() const;   // Récupère l'opération du client

        void reducePatience();   // Diminue la patience du client
        bool isPatient() const;  // Vérifie si le client est encore patient
        int getPatienceTime() const;   // Patience restante (pour l'affichage)

        virtual std::string toString() const;   // Retourne les informations du client

    private:
        int m_arrivalTime;              // Heure d'arrivée
        int m_serviceStartTime;         // Début du service
        int m_departureTime;            // Heure de départ
        AbstractOperation* m_operation; // Opération du client
        int m_patienceTime;             // Patience restante
};

#endif // ABSTRACTCLIENT_H