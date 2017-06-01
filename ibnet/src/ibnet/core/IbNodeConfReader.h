#ifndef IBNET_CORE_IBNODECONFREADER_H
#define IBNET_CORE_IBNODECONFREADER_H

#include "IbNodeConf.h"

namespace ibnet {
namespace core {

class IbNodeConfReader
{
public:
    IbNodeConfReader(void) {};
    virtual ~IbNodeConfReader(void) {};

    virtual IbNodeConf Read(void) = 0;
};

}
}

#endif // IBNET_CORE_IBNODECONFREADER_H
