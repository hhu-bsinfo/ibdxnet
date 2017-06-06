#include "SendQueues.h"

#include "MsgException.h"

namespace ibnet {
namespace msg {

SendQueues::SendQueues(uint16_t maxNumConnections, uint32_t jobQueueSizePerConnection) :
    m_writeInterests(maxNumConnections),
    m_connections(maxNumConnections)
{
    for (size_t i = 0; i < maxNumConnections; i++) {
        m_connections[i].m_nodeId = core::IbNodeId::INVALID;
        m_connections[i].m_writeInterestCount = 0;
        m_connections[i].m_flowControlData =
            std::make_shared<std::atomic<uint32_t>>(0);
        m_connections[i].m_queue = std::make_shared<
            ibnet::sys::Queue<std::shared_ptr<SendData>>>(
            jobQueueSizePerConnection);
        m_connections[i].m_aquiredQueue = false;
    }
}

SendQueues::~SendQueues(void)
{

}

bool SendQueues::PushBack(std::shared_ptr<SendData> data)
{
    uint16_t connectionId = data->m_connectionId;

    if (!m_connections[connectionId].m_queue->PushBack(data)) {
        return false;
    }

    // we have to remember the nodeId somewhere to easily get a
    // connectionId -> nodeId mapping
    m_connections[connectionId].m_nodeId = data->m_destNodeId;

    if (m_connections[connectionId].m_writeInterestCount
            .fetch_add(1, std::memory_order_relaxed) == 0) {
        m_writeInterests.PushBack(connectionId);
    }

    return true;
}

bool SendQueues::PushBackFlowControl(uint16_t nodeId, uint16_t connectionId,
        uint32_t bytes)
{
    m_connections[connectionId].m_flowControlData->fetch_add(bytes,
        std::memory_order_relaxed);

    // we have to remember the nodeId somewhere to easily get a
    // connectionId -> nodeId mapping
    m_connections[connectionId].m_nodeId = nodeId;

    if (m_connections[connectionId].m_writeInterestCount
            .fetch_add(1, std::memory_order_relaxed) == 0) {
        m_writeInterests.PushBack(connectionId);
    }

    return true;
}

bool SendQueues::Next(uint16_t& targetNodeId, uint16_t& connectionId,
        std::shared_ptr<std::atomic<uint32_t>>& flowControlData,
        std::shared_ptr<ibnet::sys::Queue<std::shared_ptr<SendData>>>& queue)
{
    if (!m_writeInterests.PopFront(connectionId)) {
        return false;
    }

    bool expected = false;
    if (!m_connections[connectionId].m_aquiredQueue.compare_exchange_strong(
            expected, true, std::memory_order_acquire)) {

        // got an interest token but the queue is already acquired
        // put interest back
        while (!m_writeInterests.PushBack(connectionId)) {
            // force push back, don't lose interest
            std::this_thread::yield();
        }

        return false;
    }

    targetNodeId = m_connections[connectionId].m_nodeId;
    flowControlData = m_connections[connectionId].m_flowControlData;
    queue = m_connections[connectionId].m_queue;

    return true;
}

void SendQueues::NodeDisconnected(uint16_t connectionId)
{
    // important: because all write interests are added to a queue, we can't
    // remove them (easily). we leave the interest in the queue but set the
    // count to 0. thus, if a thread gets the next interest in the queue
    // it has to check if there are even operations available.

    m_connections[connectionId].m_nodeId = core::IbNodeId::INVALID;
    m_connections[connectionId].m_writeInterestCount.store(0,
        std::memory_order_relaxed);
    m_connections[connectionId].m_queue->Clear();
    m_connections[connectionId].m_aquiredQueue.store(false,
        std::memory_order_relaxed);
}

void SendQueues::Finished(uint16_t connectionId, uint32_t consumedInterests)
{
    if (m_connections[connectionId].m_writeInterestCount.fetch_sub(
            consumedInterests,
            std::memory_order_relaxed) - consumedInterests > 0) {
        m_writeInterests.PushBack(connectionId);
    }

    m_connections[connectionId].m_aquiredQueue.store(false,
        std::memory_order_release);
}

}
}