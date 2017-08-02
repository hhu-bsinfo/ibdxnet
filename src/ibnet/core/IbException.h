#ifndef IBNET_CORE_IBEXCEPTION_H
#define IBNET_CORE_IBEXCEPTION_H

#include "ibnet/sys/Exception.h"

namespace ibnet {
namespace core {

/**
 * Base class for all exceptions in this namespace
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbException : public sys::Exception
{
public:
    /**
     * Constructor
     *
     * @param msg Exception message
     */
    IbException(const std::string& msg) :
            Exception(msg)
    {

    }

    /**
     * Destructor
     */
    ~IbException(void) {};
};

}
}

#endif // IBNET_CORE_IBEXCEPTION_H
