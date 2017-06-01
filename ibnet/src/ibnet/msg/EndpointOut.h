#ifndef IBNET_MSG_ENDPOINTOUT_H
#define IBNET_MSG_ENDPOINTOUT_H

#include "ibnet/msg/BufferObject.h"
#include "ibnet/sys/Queue.h"

namespace ibnet {
namespace msg {

struct EndpointOut
{
    uint16_t m_nodeId;
    sys::Queue<std::shared_ptr<BufferObject>> m_bufferQueue;
    sys::Queue<uint32_t> m_flowControlQueue;

    EndpointOut(uint16_t nodeId, uint32_t bufferQueueSize,
        uint32_t flowControlQueueSize) :
            m_nodeId(nodeId),
            m_bufferQueue(bufferQueueSize),
            m_flowControlQueue(flowControlQueueSize)
    {};

    ~EndpointOut(void)
    {

    }
};

}
}

#endif //IBNET_MSG_ENDPOINTOUT_H
