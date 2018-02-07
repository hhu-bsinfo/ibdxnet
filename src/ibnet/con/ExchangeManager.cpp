//
// Created by nothaas on 1/29/18.
//

#include "ExchangeManager.h"

#include "ibnet/sys/IllegalStateException.h"

#include "ExchangeDispatcher.h"

#define PAKET_MAGIC 0xBEEFCA4E

namespace ibnet {
namespace con {

ExchangeManager::ExchangeManager(con::NodeId ownNodeId, uint16_t socketPort) :
    ThreadLoop("ExchangeManager"),
    m_ownNodeId(ownNodeId),
    m_socket(new sys::SocketUDP(socketPort)),
    m_recvBuffer(new uint8_t[MAX_PAKET_SIZE]),
    m_noDataAvailable(false),
    m_paketTypeIdCounter(0),
    m_dispatcherLock(),
    m_dispatcher()
{
    Start();
}

ExchangeManager::~ExchangeManager()
{
    Stop();

    delete m_socket;
    delete [] m_recvBuffer;
}

ExchangeManager::PaketType ExchangeManager::GeneratePaketTypeId()
{
    if (m_paketTypeIdCounter == INVALID_PACKET_TYPE) {
        throw sys::IllegalStateException("Out of packet type ids");
    }

    std::lock_guard<std::mutex> l(m_dispatcherLock);

    PaketType type = m_paketTypeIdCounter++;

    m_dispatcher.resize(m_paketTypeIdCounter);

    return type;
}

void ExchangeManager::SendData(PaketType type, uint32_t targetIPV4,
        const void* data, uint32_t length)
{
    size_t sendSize = sizeof(PaketHeader) + length;

    if (type >= m_paketTypeIdCounter) {
        throw sys::IllegalStateException("Invalid paket type id: %d", type);
    }

    if (sendSize >= MAX_PAKET_SIZE) {
        throw sys::IllegalStateException(
            "Send data size exceeds max paket size: %d", sendSize);
    }

    uint8_t sendBuffer[MAX_PAKET_SIZE];
    auto* header = (PaketHeader*) &sendBuffer[0];

    header->m_magic = PAKET_MAGIC;
    header->m_type = type;
    header->m_sourceNodeId = m_ownNodeId;

    if (data) {
        header->m_length = length;
        memcpy(sendBuffer + sizeof(PaketHeader), data, length);
    } else {
        header->m_length = 0;
    }

    ssize_t ret = m_socket->Send(&sendBuffer, sendSize, targetIPV4,
        m_socket->GetPort());

    if (ret != static_cast<ssize_t>(sendSize)) {
        IBNET_LOG_ERROR("Sending exchg data type %d, length %d to %s failed",
            type, length, sys::AddressIPV4(targetIPV4));
    }
}

void ExchangeManager::_RunLoop()
{
    uint32_t recvAddr = 0;

    ssize_t res = m_socket->Receive(m_recvBuffer, MAX_PAKET_SIZE, &recvAddr);

    if (res < static_cast<ssize_t>(sizeof(PaketHeader))) {
        m_noDataAvailable = true;
    } else {
        auto header = (PaketHeader*) (m_recvBuffer);

        IBNET_LOG_TRACE(
            "Received paket from %s, magic 0x%X, type %d, nodeId 0x%X, "
            "length %d", sys::AddressIPV4(recvAddr), header->m_magic,
            header->m_type, header->m_sourceNodeId, header->m_length);

        if (header->m_magic == PAKET_MAGIC &&
                header->m_type < m_paketTypeIdCounter &&
                sizeof(PaketHeader) + header->m_length <= MAX_PAKET_SIZE) {

            std::lock_guard<std::mutex> l(m_dispatcherLock);

            if (!m_dispatcher[header->m_type].empty()) {

                void* data = nullptr;

                if (header->m_length > 0) {
                    data = (void*) (((uintptr_t) m_recvBuffer) +
                        sizeof(PaketHeader));
                }

                for (auto& it : m_dispatcher[header->m_type]) {
                    it->_DispatchExchangeData(recvAddr, header, data);
                }
            } else {
                IBNET_LOG_WARN("No dispatcher available for paket type %d",
                    header->m_type);
            }
        } else {
            IBNET_LOG_WARN("Received invalid paket from %s, magic 0x%X, "
                "type %d, nodeId 0x%X, length %d", sys::AddressIPV4(recvAddr),
                header->m_magic, header->m_type, header->m_sourceNodeId,
                header->m_length);
        }
    }

    if (m_noDataAvailable) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


}
}