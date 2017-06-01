#ifndef IBNET_CORE_IBCONNECTIONCREATOR_H
#define IBNET_CORE_IBCONNECTIONCREATOR_H

#include "IbDevice.h"
#include "IbConnection.h"
#include "IbProtDom.h"

namespace ibnet {
namespace core {

class IbConnectionCreator
{
public:
    IbConnectionCreator(void) {};
    virtual ~IbConnectionCreator(void) {};

    virtual std::shared_ptr<IbConnection> CreateConnection(
        uint16_t connectionId,
        std::shared_ptr<IbDevice>& device,
        std::shared_ptr<IbProtDom>& protDom) = 0;
};

}
}

#endif //IBNET_CORE_IBCONNECTIONCREATOR_H
