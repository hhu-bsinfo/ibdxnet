#ifndef IBNET_CORE_IBMEMORYEXCEPTION_H
#define IBNET_CORE_IBMEMORYEXCEPTION_H

#include "ibnet/sys/StringUtils.h"

#include "IbException.h"

namespace ibnet {
namespace core {

/**
 * Exception thrown on memory region related errors
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbMemoryException : public IbException
{
public:
    /**
     * Constructor
     *
     * @param mem Affected memory region
     * @param detail Error details
     */
    IbMemoryException(const IbMemReg& mem, const std::string& detail) :
            IbException("[" + mem.ToString()
                     + ": " + detail)
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
