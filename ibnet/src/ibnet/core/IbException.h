#ifndef IBNET_CORE_IBEXCEPTION_H
#define IBNET_CORE_IBEXCEPTION_H

#include "ibnet/sys/Exception.h"

namespace ibnet {
namespace core {

class IbException : public sys::Exception
{
public:
    IbException(const std::string& msg) :
            Exception(msg)
    {

    }

    ~IbException(void) {};
};

}
}

#endif // IBNET_CORE_IBEXCEPTION_H
