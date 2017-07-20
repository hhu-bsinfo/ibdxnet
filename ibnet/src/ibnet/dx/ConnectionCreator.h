#ifndef IBNET_DX_CONNECTIONCREATOR_H
#define IBNET_DX_CONNECTIONCREATOR_H

#include "ibnet/core/IbCompQueue.h"
#include "ibnet/core/IbConnectionCreator.h"
#include "ibnet/core/IbSharedRecvQueue.h"

namespace ibnet {
namespace dx {

/**
 * Connection creator for the message system. Creates two queue pairs
 * for each connection, one QP for data buffers, one for flow
 * control data.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class ConnectionCreator : public ibnet::core::IbConnectionCreator
{
public:
    /**
     * Constructor
     *
     * @param qpMaxRecvReqs Size of the buffer receive queue
     * @param qpFlowControlMaxRecvReqs Size of the flow control receive queue
     * @param sharedRecvQueue Shared receive queue for buffers
     * @param sharedRecvCompQueue Shared completion queue for buffers
     * @param sharedFlowControlRecvQueue Shared receive queue for FC data
     * @param sharedFlowControlRecvCompQueue Shared completion queue for FC data
     */
    ConnectionCreator(uint16_t qpMaxRecvReqs, uint16_t qpFlowControlMaxRecvReqs,
        std::shared_ptr<core::IbSharedRecvQueue> sharedRecvQueue,
        std::shared_ptr<core::IbCompQueue> sharedRecvCompQueue,
        std::shared_ptr<core::IbSharedRecvQueue> sharedFlowControlRecvQueue,
        std::shared_ptr<core::IbCompQueue> sharedFlowControlRecvCompQueue);
    ~ConnectionCreator(void);

    std::shared_ptr<core::IbConnection> CreateConnection(
        uint16_t connectionId,
        std::shared_ptr<core::IbDevice>& device,
        std::shared_ptr<core::IbProtDom>& protDom) override;

private:
    uint16_t m_qpMaxRecvReqs;
    uint16_t m_qpFlowControlMaxRecvReqs;
    std::shared_ptr<core::IbSharedRecvQueue> m_sharedRecvQueue;
    std::shared_ptr<core::IbCompQueue> m_sharedRecvCompQueue;
    std::shared_ptr<core::IbSharedRecvQueue> m_sharedFlowControlRecvQueue;
    std::shared_ptr<core::IbCompQueue> m_sharedFlowControlRecvCompQueue;
};

}
}

#endif //IBNET_DX_CONNECTIONCREATOR_H
