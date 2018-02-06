//
// Created by nothaas on 1/30/18.
//

#ifndef IBNET_MSGRC_CONNECTION_H
#define IBNET_MSGRC_CONNECTION_H

#include <infiniband/verbs.h>

#include "ibnet/core/IbProtDom.h"

#include "ibnet/con/Connection.h"

namespace ibnet {
namespace msgrc {

class Connection : public con::Connection
{
public:
    Connection(con::NodeId ownNodeId, con::ConnectionId connectionId,
        uint32_t sendBufferSize, uint16_t ibSQSize, ibv_srq* refIbSRQ,
        uint16_t ibSRQSize, ibv_cq* refIbSharedSCQ, uint16_t ibSharedSCQSize,
        ibv_cq* refIbSharedRCQ, uint16_t ibSharedRCQSize,
        core::IbProtDom* refProtDom);

    ~Connection() override;

    void CreateConnectionExchangeData(void* connectionDataBuffer,
        size_t connectionDataMaxSize, size_t* connectionDataActualSize)
        override;

    void Connect(const con::RemoteConnectionHeader& remoteConnectionHeader,
        const void* remoteConnectionData, size_t remoteConnectionDataSize)
        override;

    void Close(bool force) override;

    core::IbMemReg* GetRefSendBuffer() const {
        return m_sendBuffer;
    }

    ibv_qp* GetQP() const {
        return m_ibQP;
    }

private:
    struct RemoteConnectionData
    {
        uint32_t m_physicalQPId;
        uint32_t m_psn;
    } __attribute__((__packed__));

private:
    const uint32_t m_sendBufferSize;
    core::IbProtDom* m_refProtDom;
    core::IbMemReg* m_sendBuffer;

    ibv_qp* m_ibQP;
    uint32_t m_ibPhysicalQPId;
    uint32_t m_ibPsn;

    RemoteConnectionData m_remoteConnectionData;

    const uint16_t m_ibSQSize;

    ibv_srq* m_refIbSRQ;
    const uint16_t m_ibSRQSize;

    ibv_cq* m_refIbSharedSCQ;
    const uint16_t m_ibSharedSCQSize;

    ibv_cq* m_refIbSharedRCQ;
    const uint16_t m_ibSharedRCQSize;

private:
    void __CreateQP();
    void __SetInitStateQP();
    void __SetReadyToSend();
    void __SetReadyToRecv();
};

}
}

#endif //IBNET_MSGRC_CONNECTION_H
