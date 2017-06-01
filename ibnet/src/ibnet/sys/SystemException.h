#ifndef IBNET_SYS_SYSTEMEXCEPTION_H
#define IBNET_SYS_SYSTEMEXCEPTION_H

#include "ibnet/sys/Exception.h"

namespace ibnet {
namespace sys {

class SystemException : public Exception
{
public:
    SystemException(const std::string& msg) :
        Exception(msg)
    {

    }

    ~SystemException(void) {};
};

}
}


#endif //IBNET_SYS_SYSTEMEXCEPTION_H
