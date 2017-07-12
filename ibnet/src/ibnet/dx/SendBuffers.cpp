#include "SendBuffers.h"

#include "ibnet/sys/Logger.hpp"

namespace ibnet {
namespace dx {

SendBuffers::SendBuffers(uint32_t bufferSize, uint32_t maxConnections,
        std::shared_ptr<core::IbProtDom>& protDom)
{
    IBNET_LOG_INFO(
        "Allocating send buffer pool for {} connections, buffer size {}",
        maxConnections, bufferSize);

    for (uint32_t i = 0; i < maxConnections; i++) {
        m_buffers.push_back(protDom->Register(malloc(bufferSize),
            bufferSize, true));
    }

    for (uint32_t i = 0; i < maxConnections; i++) {
        m_flowControlBuffers.push_back(protDom->Register(
            malloc(sizeof(uint32_t)), sizeof(uint32_t), true));
    }
}

SendBuffers::~SendBuffers(void)
{

}

}
}