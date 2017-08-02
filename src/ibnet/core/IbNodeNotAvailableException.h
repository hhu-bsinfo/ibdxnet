#ifndef IBNET_CORE_IBNODENOTAVAILABLEEXCEPTION_H
#define IBNET_CORE_IBNODENOTAVAILABLEEXCEPTION_H

#include "ibnet/sys/StringUtils.h"

#include "IbException.h"

namespace ibnet {
namespace core {

/**
 * Exception thrown if a node is not available for an operation
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbNodeNotAvailableException : public IbException
{
public:
    /**
     * Constructor
     *
     * @param nodeId Target node id that is not available
     */
    IbNodeNotAvailableException(uint16_t nodeId) :
        IbException("Node " + sys::StringUtils::ToHexString(nodeId) +
                    " not available")
    {

    }

    /**
     * Destructor
     */
    ~IbNodeNotAvailableException(void) {};
};

}
}

#endif // IBNET_CORE_IBNODENOTAVAILABLEEXCEPTION_H
