#ifndef IBNET_CORE_IBSHAREDRECVQUEUE_H
#define IBNET_CORE_IBSHAREDRECVQUEUE_H

#include <memory>

#include "IbProtDom.h"

namespace ibnet {
namespace core {

/**
 * A shared receive queue can be used on multiple queue pairs.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbSharedRecvQueue
{
public:
    /**
     * Constructor
     *
     * @param protDom Pointer to a protection domain to create the queue in
     * @param size Size of the queue
     */
    IbSharedRecvQueue(std::shared_ptr<IbProtDom>& protDom, uint32_t size);

    /**
     * Destructor
     */
    ~IbSharedRecvQueue(void);

    /**
     * Get the size of the queue
     */
    uint32_t GetSize(void) const {
        return m_size;
    }

    /**
     * Get the IB queue object. Used by other parts of the package but
     * no need for the "user"
     */
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
