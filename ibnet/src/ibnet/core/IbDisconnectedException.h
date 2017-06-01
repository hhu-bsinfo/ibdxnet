#ifndef IBNET_CORE_IBDISCONNECTEDEXCEPTION_H
#define IBNET_CORE_IBDISCONNECTEDEXCEPTION_H

#include "IbException.h"

namespace ibnet {
namespace core {

/**
 * Exception thrown on node disconnect
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbDisconnectedException : public IbException
{
public:
    /**
     * Constructor
     */
    IbDisconnectedException(void) :
        IbException("Remote queue pair not available anymore")
    {

    }

    /**
     * Destructor
     */
    ~IbDisconnectedException(void) {};
};

}
}

#endif // IBNET_CORE_IBDISCONNECTEDEXCEPTION_H
