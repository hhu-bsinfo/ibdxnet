#ifndef IBNET_CORE_IBSHAREDRECVQUEUE_H
#define IBNET_CORE_IBSHAREDRECVQUEUE_H

#include <memory>

#include "IbProtDom.h"

namespace ibnet {
namespace core {

class IbSharedRecvQueue
{
public:
    IbSharedRecvQueue(std::shared_ptr<IbProtDom>& protDom, uint32_t size);
    ~IbSharedRecvQueue(void);

    uint32_t GetSize(void) const {
        return m_size;
    }

    ibv_srq* GetQueue(void) const {
        return m_srq;
    }

private:
    uint32_t m_size;
    ibv_srq* m_srq;
};

}
}

#endif //IBNET_CORE_IBSHAREDRECVQUEUE_H
