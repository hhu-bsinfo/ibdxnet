#ifndef IBNET_MSG_SENDQUEUES_H
#define IBNET_MSG_SENDQUEUES_H

#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

#include "ibnet/core/IbNodeId.h"

#include "InterestQueue.h"
#include "SendData.h"
#include "SendQueue.h"

namespace ibnet {
namespace msg {

/**
 * Central data structure to share a job queue for send jobs
 * on multiple send threads
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class SendQueues
{
public:
    /**
     * Constructor
     *
     * @param maxNumConnections Max number of connections
     * @param jobQueueSizePerConnection Job queue size per connection
     */
    SendQueues(uint16_t maxNumConnections, uint32_t jobQueueSizePerConnection);

    /**
     * Destructor
     */
    ~SendQueues(void);

    /**
     * Add a new job with data to send to the queue
     *
     * @param data SendData object with a buffer to send
     * @return True if adding successful, false if queue full
     */
    bool PushBack(SendData* data);

    /**
     * Add a new job with flow control data to send
     *
     * @param nodeId Destination node id to send to
     * @param connectionId Connection id of the destination
     * @param bytes Flow control data to send
     * @return True if adding to queue successful, false if queue full
     */
    bool PushBackFlowControl(uint16_t nodeId, uint16_t connectionId, uint32_t bytes);

    /**
     * Get the next entry in the job queue
     *
     * Flow control data is simply aggregated to a single variable per
     * connection but buffer data is added to a queue. When this
     * happens, a write interest for the connection is set. This
     * interest signals that there is data available that needs to
     * be sent to a certain destination. This allows us to add more
     * requests by other threads to the same queue at the same time
     * without disturbing the order of write interests on connections.
     *
     * @param targetNodeId Target node id to send to
     * @param connectionId Connection id of the target node
     * @param flowControlData Flow control data
     * @param queue Queue with buffer data to send
     * @return No write interests available, nothing to send
     */
    bool Next(uint16_t& targetNodeId, uint16_t& connectionId,
        std::shared_ptr<std::atomic<uint32_t>>& flowControlData,
        std::shared_ptr<ibnet::msg::SendQueue>& queue);

    /**
     * Signal the queues that a node disconnected.
     *
     * Removes any write interests to avoid queue pollution
     *
     * @param connectionId Connection id of the node that disconnected
     */
    void NodeDisconnected(uint16_t connectionId);

    /**
     * Signal the queue that a send thread finished processing jobs/interests
     * on a connection
     *
     * @param connectionId Connection id
     * @param consumedInterests Number of interests consumed on that connection
     */
    void Finished(uint16_t connectionId, uint32_t consumedInterests);

    /**
      * Enable output to an out stream
      */
    friend std::ostream &operator<<(std::ostream& os, const SendQueues& o) {
        os << "Write interests: " << std::dec <<
            o.m_writeInterests.GetElementCount();

        for (size_t i = 0; i < o.m_connections.size(); i++) {
            if (o.m_connections[i].m_nodeId != core::IbNodeId::INVALID) {
                os << std::endl << " " << i << ": 0x" << std::hex <<
                   o.m_connections[i].m_nodeId <<
                   ", " << std::dec << o.m_connections[i].m_writeInterest.load(
                    std::memory_order_relaxed) << ", " << std::dec <<
                   o.m_connections[i].m_writeInterestCount.load(
                       std::memory_order_relaxed) <<
                   ", " << o.m_connections[i].m_flowControlData->load(
                    std::memory_order_relaxed) <<
                   ", " << o.m_connections[i].m_queue->GetElementCount() <<
                   ", " << o.m_connections[i].m_aquiredQueue.load(
                    std::memory_order_relaxed);
            }
        }

        return os;
    }

private:
    ibnet::msg::InterestQueue m_writeInterests;

    struct Connection
    {
        uint16_t m_nodeId;
        std::atomic<bool> m_writeInterest;
        std::atomic<uint32_t> m_writeInterestCount;
        std::shared_ptr<std::atomic<uint32_t>> m_flowControlData;
        std::shared_ptr<ibnet::msg::SendQueue> m_queue;
        std::atomic<bool> m_aquiredQueue;
    };

    std::vector<Connection> m_connections;
};

}
}

#endif //IBNET_MSG_SENDQUEUES_H
