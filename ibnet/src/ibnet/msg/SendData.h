#ifndef IBNET_MSG_SENDDATA_H
#define IBNET_MSG_SENDDATA_H

#include <cstdint>

namespace ibnet {
namespace msg {

/**
 * Item for the SendQueues
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
struct SendData
{
public:
    uint16_t m_destNodeId;
    uint16_t m_connectionId;
    uint32_t m_size;
    void* m_buffer;
    bool m_freeAfterSend;

    /**
     * Constructor
     *
     * @param destNodeId Target node id to send the data to
     * @param connectionId Id of the connection to write the data to
     * @param size Size of the data (bytes)
     * @param buffer Pointer to allocated buffer with data to send
     * @param freeAfterSend True to free the buffer after done sending
     */
    SendData(uint16_t destNodeId, uint16_t connectionId, uint32_t size,
            void* buffer, bool freeAfterSend) :
        m_destNodeId(destNodeId),
        m_connectionId(connectionId),
        m_size(size),
        m_buffer(buffer),
        m_freeAfterSend(freeAfterSend)
    {};
};

}
}

#endif //IBNET_MSG_SENDDATA_H
