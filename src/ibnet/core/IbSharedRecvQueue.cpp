#include "IbSharedRecvQueue.h"

#include "ibnet/sys/Logger.hpp"

namespace ibnet {
namespace core {

IbSharedRecvQueue::IbSharedRecvQueue(std::shared_ptr<IbProtDom>& protDom,
        uint32_t size) :
    m_size(size),
    m_srq(nullptr)
{
    struct ibv_srq_init_attr attr;

    memset(&attr, 0, sizeof(attr));

    attr.attr.max_sge = 1;
    attr.attr.max_wr = size;

    IBNET_LOG_TRACE("ibv_create_srq, size {}", size);
    m_srq = ibv_create_srq(protDom->GetIBProtDom(), &attr);

    if (m_srq == nullptr) {
        IBNET_LOG_ERROR("Creating shared receive queue failed: {}",
            strerror(errno));
        throw IbException("Creating shared receive queue failed");
    }
}

IbSharedRecvQueue::~IbSharedRecvQueue(void)
{
    ibv_destroy_srq(m_srq);
}

}
}