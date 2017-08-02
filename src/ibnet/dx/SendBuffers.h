#ifndef IBNET_DX_SENDBUFFERS_H
#define IBNET_DX_SENDBUFFERS_H

#include "ibnet/core/IbProtDom.h"

namespace ibnet {
namespace dx {

/**
 * Provide one send buffer per connection. All messages are written/serialized
 * into the buffer(s).
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 14.07.2017
 */
class SendBuffers
{
public:
    /**
     * Constructor
     *
     * @param bufferSize Size of a single outgoing send buffer
     * @param maxConnections Maximum number of connections
     * @param protDom Protection domain to register buffers at
     */
    SendBuffers(uint32_t bufferSize, uint32_t maxConnections,
        std::shared_ptr<core::IbProtDom>& protDom);

    /**
     * Destructor
     */
    ~SendBuffers(void);

    /**
     * Get a send buffer from the pool
     *
     * @param connectionId Connection id to get the send buffer of
     * @return Send buffer associated with the connection
     */
    core::IbMemReg* GetBuffer(uint32_t connectionId) {
        return m_buffers[connectionId];
    }

    /**
     * Get a flow control buffer from the pool
     *
     * @param connectionId Connection id to get the FC send buffer of
     * @return FC send buffer associated with the connection
     */
    core::IbMemReg* GetFlowControlBuffer(uint32_t connectionId) {
        return m_flowControlBuffers[connectionId];
    }

private:
    std::vector<core::IbMemReg*> m_buffers;
    std::vector<core::IbMemReg*> m_flowControlBuffers;
};

}
}

#endif //PROJECT_SENDBUFFERS_H
