#ifndef IBNET_CORE_IBTIMEOUTEXCEPTION_H
#define IBNET_CORE_IBTIMEOUTEXCEPTION_H

#include "ibnet/sys/StringUtils.h"

#include "IbException.h"

namespace ibnet {
namespace core {

class IbTimeoutException : public IbException
{
public:
    IbTimeoutException(uint16_t nodeId, const std::string& detail) :
            IbException("Timeout with node " +
                    sys::StringUtils::ToHexString(nodeId) + ": " + detail)
    {

    }

    ~IbTimeoutException(void) {};
};

}
}

#endif // IBNET_CORE_IBTIMEOUTEXCEPTION_H
