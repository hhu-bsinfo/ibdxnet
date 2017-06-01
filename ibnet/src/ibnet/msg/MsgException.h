#ifndef IBNET_MSG_MSGEXCEPTION_H
#define IBNET_MSG_MSGEXCEPTION_H

#include "ibnet/sys/Exception.h"

namespace ibnet {
namespace msg {

class MsgException : public sys::Exception
{
public:
    MsgException(const std::string& msg) :
            Exception(msg)
    {

    }

    ~MsgException(void) {};
};

}
}

#endif // IBNET_MSG_MSGEXCEPTION_H
