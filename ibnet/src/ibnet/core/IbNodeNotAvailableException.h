#ifndef IBNET_CORE_IBNODENOTAVAILABLEEXCEPTION_H
#define IBNET_CORE_IBNODENOTAVAILABLEEXCEPTION_H

#include "ibnet/sys/StringUtils.h"

#include "IbException.h"

namespace ibnet {
namespace core {

class IbNodeNotAvailableException : public IbException
{
public:
    IbNodeNotAvailableException(uint16_t nodeId) :
        IbException("Node " + sys::StringUtils::ToHexString(nodeId) +
                    " not available")
    {

    }

    ~IbNodeNotAvailableException(void) {};
};

}
}

#endif // IBNET_CORE_IBNODENOTAVAILABLEEXCEPTION_H
