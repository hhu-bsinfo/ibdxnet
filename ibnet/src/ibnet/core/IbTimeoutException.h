#ifndef IBNET_CORE_IBTIMEOUTEXCEPTION_H
#define IBNET_CORE_IBTIMEOUTEXCEPTION_H

#include "ibnet/sys/StringUtils.h"

#include "IbException.h"

namespace ibnet {
namespace core {

/**
 * Exception thrown on operation timeout
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbTimeoutException : public IbException
{
public:
    /**
     * Constructor
     *
     * @param nodeId Destination node id which could not be reached
     * @param detail Information about the timeout
     */
    IbTimeoutException(uint16_t nodeId, const std::string& detail) :
            IbException("Timeout with node " +
                    sys::StringUtils::ToHexString(nodeId) + ": " + detail)
    {

    }

    /**
     * Destructor
     */
    ~IbTimeoutException(void) {};
};

}
}

#endif // IBNET_CORE_IBTIMEOUTEXCEPTION_H
