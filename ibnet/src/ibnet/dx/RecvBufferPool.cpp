#include "RecvBufferPool.h"

#include "ibnet/sys/Logger.hpp"

namespace ibnet {
namespace dx {

RecvBufferPool::RecvBufferPool(uint64_t initialTotalPoolSize,
                               uint32_t recvBufferSize, uint32_t flowControlQueueSize,
                               std::shared_ptr<core::IbProtDom>& protDom) :
    m_bufferPoolSize(initialTotalPoolSize / recvBufferSize),
    m_bufferSize(recvBufferSize),
    m_numFlowControlBuffers(flowControlQueueSize),
    m_dataBuffersFront(0),
    m_dataBuffersBack(
        (uint32_t) (initialTotalPoolSize / recvBufferSize - 1)),
    m_dataBuffersBackRes(
        (uint32_t) (initialTotalPoolSize / recvBufferSize - 1)),
    m_protDom(protDom)
{
    IBNET_LOG_INFO("Alloc {} data buffers, size {} each",
        m_bufferPoolSize, recvBufferSize);
    m_dataBuffers = new core::IbMemReg*[m_bufferPoolSize];
    for (uint32_t i = 0; i < m_bufferPoolSize; i++) {
        m_dataBuffers[i] = m_protDom->Register(
            malloc(recvBufferSize), recvBufferSize, true);
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

    uint32_t front = m_dataBuffersFront.load(std::memory_order_relaxed);
    uint32_t back;

    while (true) {
        back = m_dataBuffersBack.load(std::memory_order_relaxed);

        if (front % m_bufferPoolSize == back % m_bufferPoolSize) {
            // TODO replace with spinning
            IBNET_LOG_WARN("Insufficient pooled incoming buffers. "
                " Allocating temporary buffer...");
            buffer = m_protDom->Register(malloc(m_bufferSize), m_bufferSize, true);
            break;
        }

        buffer = m_dataBuffers[front % m_bufferPoolSize];

        m_dataBuffersFront.fetch_add(1, std::memory_order_release);
        break;
    }

    return buffer;
}

void RecvBufferPool::ReturnBuffer(core::IbMemReg* buffer)
{
    uint32_t backRes = m_dataBuffersBackRes.load(std::memory_order_relaxed);
    uint32_t front;

    while (true) {
        front = m_dataBuffersFront.load(std::memory_order_relaxed);

        if (backRes + 1 % m_bufferPoolSize == front % m_bufferPoolSize) {
            // TODO destroy and cleanup buffer
            IBNET_LOG_ERROR("Memory leak, returning temporary buffer");
            break;
        }

        if (m_dataBuffersBackRes.compare_exchange_weak(backRes, backRes + 1,
                std::memory_order_relaxed)) {
            m_dataBuffers[backRes % m_bufferPoolSize] = buffer;

            // if two buffers are returned at the same time, the first return
            // could be interrupt by a second return. the reserve of the first
            // return is already completed but the back pointer is not updated.
            // the second return reserves and updates the back pointer. now,
            // the back pointer is pointing to the first returns reserve which
            // might not be completed, yet.
            // solution: the second return has to wait for the first return
            // to complete, both, the reservation and updating of the back
            // pointer before it can update the back pointer as well
            while (!m_dataBuffersBack.compare_exchange_weak(backRes,
                    backRes + 1, std::memory_order_release)) {
                std::this_thread::yield();
            }

            break;
        }
    }
}

core::IbMemReg* RecvBufferPool::GetFlowControlBuffer(void)
{
    core::IbMemReg* buffer = NULL;

    if (!m_flowControlBuffers.empty()) {
        buffer = m_flowControlBuffers.back();
        m_flowControlBuffers.pop_back();
    } else {
        IBNET_LOG_ERROR("Out of flow control buffers");
    }

    return buffer;
}

}
}