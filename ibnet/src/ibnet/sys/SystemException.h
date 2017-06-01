#ifndef IBNET_SYS_SYSTEMEXCEPTION_H
#define IBNET_SYS_SYSTEMEXCEPTION_H

#include "ibnet/sys/Exception.h"

namespace ibnet {
namespace sys {

/**
 * Common exception that can be thrown in the sys namespace
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class SystemException : public Exception
{
public:
    /**
     * Constructor
     *
     * @param msg Exception message
     */
    SystemException(const std::string& msg) :
        Exception(msg)
    {

    }

    /**
     * Destructor
     */
    ~SystemException(void) {};
};

}
}


#endif //IBNET_SYS_SYSTEMEXCEPTION_H
