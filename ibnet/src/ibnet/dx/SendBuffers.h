#ifndef IBNET_DX_SENDBUFFERS_H
#define IBNET_DX_SENDBUFFERS_H

#include "ibnet/core/IbProtDom.h"

namespace ibnet {
namespace dx {

/**
 * Provide one send buffer per connection. All messages are written/serialized
 * into the buffer(s). Doesn't require any synchronization because each
 * SendThread will acquire a connection exclusively before working with it.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 14.07.2017
 */
class SendBuffers
{
public:
    SendBuffers(uint32_t bufferSize, uint32_t maxConnections, std::shared_ptr<core::IbProtDom>& protDom);

    ~SendBuffers(void);

    core::IbMemReg* GetBuffer(uint32_t connectionId) {
        return m_buffers[connectionId];
    }

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
