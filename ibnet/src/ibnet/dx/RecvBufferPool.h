#ifndef IBNET_DX_RECVBUFFERPOOL_H
#define IBNET_DX_RECVBUFFERPOOL_H

#include <mutex>

#include "ibnet/core/IbProtDom.h"

namespace ibnet {
namespace dx {

/**
 * Buffer pool for incoming data that are registered with a protection domain
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class RecvBufferPool
{
public:
    /**
     * Constructor
     *
     * @param bufferSize Size of the send buffer in bytes
     * @param protDom Protection domain to register all buffers at
     */
    RecvBufferPool(uint32_t bufferSize, uint32_t flowControlQueueSize, std::shared_ptr<core::IbProtDom>& protDom);

    /**
     * Destructor
     */
    ~RecvBufferPool(void);

    /**
     * Get a buffer from the pool. The buffer returned will be the biggest
     * buffer currently available. If the pool is empty, new buffers are
     * allocated to avoid running dry.
     *
     * @return Buffer
     */
    core::IbMemReg* GetBuffer(void);

    /**
     * Return a buffer to be reused
     *
     * @param buffer Buffer to return
     */
    void ReturnBuffer(core::IbMemReg* buffer);

    // TODO doc
    core::IbMemReg* GetFlowControlBuffer(void);

    // TODO doc
    void ReturnFlowControlBuffer(core::IbMemReg* buffer);

private:
    const uint32_t m_largeBufferPoolSize;
    const uint32_t m_mediumBufferPoolSize;
    const uint32_t m_smallBufferPoolSize;

    const uint32_t m_largeBufferPoolFactor;
    const uint32_t m_mediumBufferPoolFactor;
    const uint32_t m_smallBufferPoolFactor;

    const uint32_t m_largeBufferSize;
    const uint32_t m_mediumBufferSize;
    const uint32_t m_smallBufferSize;

    const uint32_t m_bufferSize;

    const uint32_t m_numFlowControlBuffers;

    std::vector<core::IbMemReg*> m_largeBufferPool;
    std::vector<core::IbMemReg*> m_mediumBufferPool;
    std::vector<core::IbMemReg*> m_smallBufferPool;

    std::vector<core::IbMemReg*> m_flowControlBuffers;

    std::mutex m_lock;
    std::mutex m_flowControlLock;

    std::shared_ptr<core::IbProtDom> m_protDom;
};

}
}

#endif //IBNET_DX_RECVBUFFERPOOL_H
