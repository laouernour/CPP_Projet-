#ifndef CONSULTATION_H
#define CONSULTATION_H

#include "client/AbstractOperation.h"

class Consultation : public AbstractOperation
{
    public:
        Consultation(int serviceTime);

        bool isUrgent() const override;
        std::string toString() const override;
};

#endif // CONSULTATION_H
