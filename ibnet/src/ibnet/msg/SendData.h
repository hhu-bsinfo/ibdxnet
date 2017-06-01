#ifndef IBNET_MSG_SENDDATA_H
#define IBNET_MSG_SENDDATA_H

#include <cstdint>

namespace ibnet {
namespace msg {

struct SendData
{
public:
    uint16_t m_destNodeId;
    uint16_t m_connectionId;
    uint32_t m_size;
    void* m_buffer;

    SendData(uint16_t destNodeId, uint16_t connectionId, uint32_t size,
            void* buffer) :
        m_destNodeId(destNodeId),
        m_connectionId(connectionId),
        m_size(size),
        m_buffer(buffer)
    {};
};

}
}

#endif //IBNET_MSG_SENDDATA_H
