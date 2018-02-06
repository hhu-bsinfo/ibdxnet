//
// Created by nothaas on 1/29/18.
//

#ifndef IBNET_CON_EXCHANGEDISPATCHER_H
#define IBNET_CON_EXCHANGEDISPATCHER_H

#include "ExchangeManager.h"

namespace ibnet {
namespace con {

// forward declaration for friendship
class ExchangeManager;

class ExchangeDispatcher
{
public:
    friend class ExchangeManager;

protected:
    ExchangeDispatcher() = default;

    virtual ~ExchangeDispatcher() = default;

    virtual void _DispatchExchangeData(uint32_t sourceIPV4,
        const ExchangeManager::PaketHeader* paketHeader,
        const void* data) = 0;
};

}
}

#endif //IBNET_CON_JOBDISPATCHER_H
