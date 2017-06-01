#ifndef IBNET_CORE_IBCONNECTION_H
#define IBNET_CORE_IBCONNECTION_H

#include <infiniband/verbs.h>

#include "IbCompQueue.h"
#include "IbNodeNotAvailableException.h"
#include "IbQueuePair.h"
#include "IbRemoteInfo.h"

namespace ibnet {
namespace core {

class IbConnection
{
public:
    IbConnection(
        uint16_t connectionId,
        std::shared_ptr<IbDevice>& device,
        std::shared_ptr<IbProtDom>& protDom);
    ~IbConnection(void);

    uint16_t GetConnectionId(void) const {
        return m_connectionId;
    }

    void AddQp(std::shared_ptr<IbSharedRecvQueue>& sharedRecvQueue,
        std::shared_ptr<IbCompQueue>& sharedRecvCompQueue,
        uint16_t maxRecvReqs,
        uint16_t maxSendReqs);

    std::shared_ptr<IbQueuePair>& GetQp(uint32_t idx) {
        if (!m_isConnected) {
            throw IbNodeNotAvailableException(m_remoteInfo.GetNodeId());
        }

        return m_qps.at(idx);
    }

    const std::vector<std::shared_ptr<IbQueuePair>>& GetQps(void) const {
        return m_qps;
    }

    bool IsConnected(void) const {
        return m_isConnected;
    }

    void Connect(const IbRemoteInfo& remoteInfo);

    void Close(bool force);

    friend std::ostream &operator<<(std::ostream& os, const IbConnection& o) {
        return os << "Connected: " << o.m_isConnected <<
                     ", RemoteInfo: " << o.m_remoteInfo;
    }

private:
    uint16_t m_connectionId;
    std::shared_ptr<IbDevice> m_device;
    std::shared_ptr<IbProtDom> m_protDom;

    IbRemoteInfo m_remoteInfo;
    std::vector<std::shared_ptr<IbQueuePair>> m_qps;
    std::atomic<bool> m_isConnected;
};

}
}

#endif // IBNET_CORE_IBCONNECTION_H
