#include "RecvBufferPool.h"

#include "ibnet/sys/Logger.hpp"

namespace ibnet {
namespace dx {

RecvBufferPool::RecvBufferPool(uint32_t bufferSize, uint32_t flowControlQueueSize, std::shared_ptr<core::IbProtDom>& protDom) :
    m_largeBufferPoolSize(16),
    m_mediumBufferPoolSize(16),
    m_smallBufferPoolSize(5 * 1024),
    m_largeBufferPoolFactor(2),
    m_mediumBufferPoolFactor(4),
    m_smallBufferPoolFactor(32),
    m_bufferSize(bufferSize),
    m_largeBufferSize(bufferSize / m_largeBufferPoolFactor),
    m_mediumBufferSize(bufferSize / m_mediumBufferPoolFactor),
    m_smallBufferSize(bufferSize / m_smallBufferPoolFactor),
    m_numFlowControlBuffers(flowControlQueueSize),
    m_protDom(protDom)
{
    IBNET_LOG_INFO("Alloc {} large buffers, size {} each",
        m_largeBufferPoolSize, m_largeBufferSize);

    for (uint32_t i = 0; i < m_largeBufferPoolSize; i++) {
        m_largeBufferPool.push_back(m_protDom->Register(
            malloc(m_largeBufferSize), m_largeBufferSize, true));
    }

    IBNET_LOG_INFO("Alloc {} medium buffers, size {} each",
        m_mediumBufferPoolSize, m_mediumBufferSize);

    for (uint32_t i = 0; i < m_mediumBufferPoolSize; i++) {
        m_mediumBufferPool.push_back(m_protDom->Register(
            malloc(m_mediumBufferSize), m_mediumBufferSize, true));
    }

    IBNET_LOG_INFO("Alloc {} small buffers, size {} each",
        m_smallBufferPoolSize, m_smallBufferSize);

    for (uint32_t i = 0; i < m_smallBufferPoolSize; i++) {
        m_smallBufferPool.push_back(m_protDom->Register(
            malloc(m_smallBufferSize), m_smallBufferSize, true));
    }

    IBNET_LOG_INFO("Alloc {} fc buffers buffers", m_numFlowControlBuffers);

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

    if (!m_largeBufferPool.empty()) {
        buffer = m_largeBufferPool.back();
        m_largeBufferPool.pop_back();
    } else if (!m_mediumBufferPool.empty()) {
        buffer = m_mediumBufferPool.back();
        m_mediumBufferPool.pop_back();
    } else if (!m_smallBufferPool.empty()) {
        buffer = m_smallBufferPool.back();
        m_smallBufferPool.pop_back();
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

    if (buffer->GetSize() == m_largeBufferSize) {
        if (m_largeBufferPool.size() < m_largeBufferPoolSize) {
            m_largeBufferPool.push_back(buffer);
        } else {
            // TODO destroy and cleanup buffer
            IBNET_LOG_ERROR("Memory leak, returning temporary buffer");
        }
    } else if (buffer->GetSize() == m_mediumBufferSize) {
        if (m_mediumBufferPool.size() < m_mediumBufferPoolSize) {
            m_mediumBufferPool.push_back(buffer);
        } else {
            // TODO destroy and cleanup buffer
            IBNET_LOG_ERROR("Memory leak, returning temporary buffer");
        }
    } else if (buffer->GetSize() == m_smallBufferSize) {
        if (m_smallBufferPool.size() < m_smallBufferPoolSize) {
            m_smallBufferPool.push_back(buffer);
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