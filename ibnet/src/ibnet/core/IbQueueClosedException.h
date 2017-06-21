#ifndef IBNET_CORE_IBQUEUECLOSEDEXCEPTION_H
#define IBNET_CORE_IBQUEUECLOSEDEXCEPTION_H

#include "ibnet/sys/StringUtils.h"

#include "IbException.h"

namespace ibnet {
namespace core {

/**
 * Exception thrown if a queue is closed
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 21.06.2017
 */
class IbQueueClosedException : public IbException
{
public:
    /**
     * Constructor
     */
    IbQueueClosedException(void) :
            IbException("")
    {

    }

    /**
     * Destructor
     */
    ~IbQueueClosedException(void) {};
};

}
}

#endif // IBNET_CORE_IBQUEUECLOSEDEXCEPTION_H
