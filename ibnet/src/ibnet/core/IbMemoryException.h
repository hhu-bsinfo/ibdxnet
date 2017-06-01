#ifndef IBNET_CORE_IBMEMORYEXCEPTION_H
#define IBNET_CORE_IBMEMORYEXCEPTION_H

#include "ibnet/sys/StringUtils.h"

#include "IbException.h"

namespace ibnet {
namespace core {

class IbMemoryException : public IbException
{
public:
    IbMemoryException(const IbMemReg& mem, const std::string& detail) :
            IbException("[" + mem.ToString()
                     + ": " + detail)
    {

    }

    ~IbTimeoutException(void) {};
};

}
}

#endif // IBNET_CORE_IBTIMEOUTEXCEPTION_H
