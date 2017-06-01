#ifndef IBNET_MSG_SENDQUEUES_H
#define IBNET_MSG_SENDQUEUES_H

#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

#include "ibnet/core/IbNodeId.h"
#include "ibnet/sys/Queue.h"

#include "SendData.h"

namespace ibnet {
namespace msg {

class SendQueues
{
public:
    SendQueues(uint16_t maxNumConnections, uint32_t jobQueueSizePerConnection);

    ~SendQueues(void);

    bool PushBack(std::shared_ptr<SendData> data);

    bool PushBackFlowControl(uint16_t nodeId, uint16_t connectionId, uint32_t bytes);

    bool Next(uint16_t& targetNodeId, uint16_t& connectionId,
        std::shared_ptr<std::atomic<uint32_t>>& flowControlData,
        std::shared_ptr<ibnet::sys::Queue<std::shared_ptr<SendData>>>& queue);

    void NodeDisconnected(uint16_t connectionId);

    void Finished(uint16_t connectionId, uint32_t consumedInterests);

    friend std::ostream &operator<<(std::ostream& os, const SendQueues& o) {
        os << "Write interests: " << std::dec <<
            o.m_writeInterests.GetElementCount();

        for (size_t i = 0; i < o.m_connections.size(); i++) {
            if (o.m_connections[i].m_nodeId != core::IbNodeId::INVALID) {
                os << std::endl << i << ": 0x" << std::hex <<
                   o.m_connections[i].m_nodeId <<
                   ", " << std::dec << o.m_connections[i].m_writeInterestCount
                   <<
                   ", " << o.m_connections[i].m_flowControlData->load(
                    std::memory_order_relaxed) <<
                   ", " << o.m_connections[i].m_queue->GetElementCount() <<
                   ", " << o.m_connections[i].m_aquiredQueue;
            }
        }

        return os;
    }

private:
    sys::Queue<uint16_t> m_writeInterests;

    struct Connection
    {
        uint16_t m_nodeId;
        std::atomic<uint32_t> m_writeInterestCount;
        std::shared_ptr<std::atomic<uint32_t>> m_flowControlData;
        std::shared_ptr<ibnet::sys::Queue<std::shared_ptr<SendData>>> m_queue;
        std::atomic<bool> m_aquiredQueue;
    };

    std::vector<Connection> m_connections;
};

}
}

#endif //IBNET_MSG_SENDQUEUES_H
