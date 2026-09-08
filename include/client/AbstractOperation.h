#ifndef ABSTRACTOPERATION_H
#define ABSTRACTOPERATION_H

#include <string>   // Permet d'utiliser std::string

class AbstractOperation
{
    public:
        AbstractOperation(int serviceTime);   // Crée une opération avec un temps de service
        virtual ~AbstractOperation();        // Libère la mémoire

        int getServiceTime() const;           // Récupère le temps de service

        virtual bool isUrgent() const = 0;    // Vérifie si l'opération est urgente

        virtual std::string toString() const = 0;   // Retourne les informations de l'opération

    private:
        int m_serviceTime;   // Temps nécessaire pour réaliser l'opération
};

#endif // ABSTRACTOPERATION_H