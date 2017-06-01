#ifndef IBNET_CORE_IBQUEUEFULLEXCEPTION_H
#define IBNET_CORE_IBQUEUEFULLEXCEPTION_H

#include "ibnet/sys/StringUtils.h"

#include "IbException.h"

namespace ibnet {
namespace core {

/**
 * Exception thrown if a queue is full
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbQueueFullException : public IbException
{
public:
    /**
     * Constructor
     *
     * @param str Further error information
     */
    IbQueueFullException(const std::string& str) :
            IbException(str)
    {

    }

    /**
     * Destructor
     */
    ~IbQueueFullException(void) {};
};

}
}

#endif // IBNET_CORE_IBQUEUEFULLEXCEPTION_H
