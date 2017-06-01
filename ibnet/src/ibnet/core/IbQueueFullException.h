#ifndef IBNET_CORE_IBQUEUEFULLEXCEPTION_H
#define IBNET_CORE_IBQUEUEFULLEXCEPTION_H

#include "ibnet/sys/StringUtils.h"

#include "IbException.h"

namespace ibnet {
namespace core {

class IbQueueFullException : public IbException
{
public:
    IbQueueFullException(const std::string& str) :
            IbException(str)
    {

    }

    ~IbQueueFullException(void) {};
};

}
}

#endif // IBNET_CORE_IBQUEUEFULLEXCEPTION_H
