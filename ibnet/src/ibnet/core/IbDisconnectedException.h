#ifndef IBNET_CORE_IBDISCONNECTEDEXCEPTION_H
#define IBNET_CORE_IBDISCONNECTEDEXCEPTION_H

#include "IbException.h"

namespace ibnet {
namespace core {

class IbDisconnectedException : public IbException
{
public:
    IbDisconnectedException(void) :
        IbException("Remote queue pair not available anymore")
    {

    }

    ~IbDisconnectedException(void) {};
};

}
}

#endif // IBNET_CORE_IBDISCONNECTEDEXCEPTION_H
