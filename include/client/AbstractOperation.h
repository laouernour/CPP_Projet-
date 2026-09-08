#ifndef ABSTRACTOPERATION_H
#define ABSTRACTOPERATION_H

#include <string>

class AbstractOperation
{
    public:
        AbstractOperation(int serviceTime);
        virtual ~AbstractOperation();

        int getServiceTime() const;
        virtual bool isUrgent() const = 0;
        virtual std::string toString() const = 0;

    private:
        int m_serviceTime;
};

#endif // ABSTRACTOPERATION_H
