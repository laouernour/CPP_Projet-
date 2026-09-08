#ifndef ABSTRACTCLIENT_H
#define ABSTRACTCLIENT_H

#include <string>

#include "client/AbstractOperation.h"

/**
 * On a besoin de savoir quand un client arrive, quand le service commence et se
 * termine. A la creation du client, seul son temps d'arrivee est defini.
 * Le client est proprietaire de son operation (il la detruit).
 */
class AbstractClient
{
    public:
        AbstractClient(int arrivalTime, AbstractOperation* operation, int patienceTime);
        virtual ~AbstractClient();

        virtual bool isPriority() const = 0;

        int getArrivalTime() const;

        int getDepartureTime() const;
        void setDepartureTime(int departureTime);

        int getServiceStartTime() const;
        void setServiceStartTime(int serviceStartTime);

        AbstractOperation* getOperation() const;

        void reducePatience();
        bool isPatient() const;

        virtual std::string toString() const;

    private:
        int m_arrivalTime;
        int m_serviceStartTime;
        int m_departureTime;
        AbstractOperation* m_operation;
        int m_patienceTime;
};

#endif // ABSTRACTCLIENT_H
