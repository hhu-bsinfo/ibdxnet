#include "RecvBufferPool.h"

#include "ibnet/sys/Logger.hpp"

namespace ibnet {
namespace dx {

RecvBufferPool::RecvBufferPool(uint32_t initialTotalPoolSize,
                               uint32_t recvBufferSize, uint32_t flowControlQueueSize,
                               std::shared_ptr<core::IbProtDom>& protDom) :
    m_bufferPoolSize(initialTotalPoolSize / recvBufferSize),
    m_bufferSize(recvBufferSize),
    m_numFlowControlBuffers(flowControlQueueSize),
    m_protDom(protDom)
{
    IBNET_LOG_INFO("Alloc {} data buffers, size {} each",
        m_bufferPoolSize, recvBufferSize);
    for (uint32_t i = 0; i < m_bufferPoolSize; i++) {
        m_dataBuffers.push_back(m_protDom->Register(
            malloc(recvBufferSize), recvBufferSize, true));
    }

    IBNET_LOG_INFO("Alloc {} fc buffers", m_numFlowControlBuffers);

    for (uint32_t i = 0; i < m_numFlowControlBuffers; i++) {
        m_flowControlBuffers.push_back(m_protDom->Register(
            malloc(4), 4, true));
    }
}

RecvBufferPool::~RecvBufferPool(void)
{

}

core::IbMemReg* RecvBufferPool::GetBuffer(void)
{
    core::IbMemReg* buffer = NULL;

    m_lock.lock();

    if (!m_dataBuffers.empty()) {
        buffer = m_dataBuffers.back();
        m_dataBuffers.pop_back();
    } else {
        IBNET_LOG_WARN("Insufficient pooled incoming buffers. "
            " Allocating temporary buffer...");
        buffer = m_protDom->Register(malloc(m_bufferSize), m_bufferSize, true);
    }

    m_lock.unlock();

    return buffer;
}

void RecvBufferPool::ReturnBuffer(core::IbMemReg* buffer)
{
    m_lock.lock();

    if (buffer->GetSize() == m_bufferSize) {
        if (m_dataBuffers.size() < m_bufferPoolSize) {
            m_dataBuffers.push_back(buffer);
        } else {
            // TODO destroy and cleanup buffer
            IBNET_LOG_ERROR("Memory leak, returning temporary buffer");
        }
    } else {
        // TODO destroy and cleanup buffer
        IBNET_LOG_ERROR("Encountered buffer with invalid size {}, cannot be assigned to a pool",
            buffer->GetSize());
    }

    m_lock.unlock();
}

core::IbMemReg* RecvBufferPool::GetFlowControlBuffer(void)
{
    core::IbMemReg* buffer = NULL;

    m_flowControlLock.lock();

    if (!m_flowControlBuffers.empty()) {
        buffer = m_flowControlBuffers.back();
        m_flowControlBuffers.pop_back();
    } else {
        IBNET_LOG_ERROR("Out of flow control buffers");
    }

    m_flowControlLock.unlock();

    return buffer;
}

void RecvBufferPool::ReturnFlowControlBuffer(core::IbMemReg* buffer)
{
    m_flowControlLock.lock();

    m_flowControlBuffers.push_back(buffer);

    m_flowControlLock.unlock();
}

}
}