//
// Created by ruhland on 1/30/18.
//

#include "Connection.h"

#include "ibnet/sys/Logger.hpp"
#include "ibnet/sys/IllegalStateException.h"

#define DEFAULT_IB_PORT 1
#define IB_QOS_LEVEL 0

namespace ibnet {
namespace msgud {

Connection::Connection(uint16_t ownNodeId, uint16_t connectionId,
        uint32_t sendBufferSize, uint32_t physicalQPId,
        core::IbProtDom *refProtDom) :
    con::Connection(ownNodeId, connectionId),
    m_sendBufferSize(sendBufferSize),
    m_refProtDom(refProtDom),
    m_sendBuffer(nullptr),
    m_ibAddressHandle(nullptr),
    m_ownPhysicalQPId(physicalQPId)
{
    IBNET_LOG_TRACE_FUNC;

    IBNET_LOG_DEBUG("Allocate send buffer, size %d for connection id 0x%X",
        m_sendBufferSize, connectionId);

    m_sendBuffer = new core::IbMemReg(
        aligned_alloc(static_cast<size_t>(getpagesize()), m_sendBufferSize),
        m_sendBufferSize, true);

    m_refProtDom->Register(m_sendBuffer);
}

Connection::~Connection()
{
    if(m_ibAddressHandle != nullptr) {
        delete m_ibAddressHandle;
    }
}

void Connection::CreateConnectionExchangeData(void* connectionDataBuffer,
        size_t connectionDataMaxSize, size_t* connectionDataActualSize)
{
    if (connectionDataMaxSize < sizeof(RemoteConnectionData)) {
        throw sys::IllegalStateException("Buffer too small");
    }

    auto* data = static_cast<RemoteConnectionData*>(connectionDataBuffer);

    data->m_physicalQPId = m_ownPhysicalQPId;

    *connectionDataActualSize = sizeof(RemoteConnectionData);
}

void Connection::Connect(
        const con::RemoteConnectionHeader& remoteConnectionHeader,
        const void* remoteConnectionData, size_t remoteConnectionDataSize)
{
    if (remoteConnectionDataSize < sizeof(RemoteConnectionData)) {
        throw sys::IllegalStateException("Buffer too small");
    }

    auto* data = static_cast<const RemoteConnectionData*>(remoteConnectionData);

    m_remoteConnectionHeader = remoteConnectionHeader;
    m_remoteConnectionData = *data;

    m_remotePhysicalQPId = m_remoteConnectionData.m_physicalQPId;
    m_ibAddressHandle = new ibnet::core::IbAddressHandle(*m_refProtDom,
            m_remoteConnectionHeader.m_lid, 0, 0, 0, DEFAULT_IB_PORT);
}

void Connection::Close(bool force)
{
    IBNET_LOG_TRACE_FUNC;

    delete m_ibAddressHandle;
    m_ibAddressHandle = nullptr;
}

}
}
