#ifndef IBNET_MSG_MSGEXCEPTION_H
#define IBNET_MSG_MSGEXCEPTION_H

#include "ibnet/sys/Exception.h"

namespace ibnet {
namespace msg {

/**
 * Common exception for the msg namespace
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class MsgException : public sys::Exception
{
public:
    /**
     * Constructor
     *
     * @param msg Exception message
     */
    MsgException(const std::string& msg) :
            Exception(msg)
    {

    }

    /**
     * Destructor
     */
    ~MsgException(void) {};
};

}
}

#endif // IBNET_MSG_MSGEXCEPTION_H
