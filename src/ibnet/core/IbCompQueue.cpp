#include "IbCompQueue.h"

#include "ibnet/sys/Logger.hpp"

#include "IbDisconnectedException.h"

namespace ibnet {
namespace core {

IbCompQueue::IbCompQueue(std::shared_ptr<IbDevice>& device, uint16_t size) :
    m_size(size),
    m_cq(nullptr),
    m_firstWc(true),
    m_outstandingComps(size)
{
    IBNET_LOG_TRACE("ibv_create_cq, size {}", size);
    m_cq = ibv_create_cq(
            device->GetIBContext(),
            size,
            nullptr,
            nullptr,
            0);

    if (m_cq == nullptr) {
        IBNET_LOG_ERROR("Creating completion queue failed: {}", strerror(errno));
        throw IbException("Creating completion queue failed");
    }
}

IbCompQueue::~IbCompQueue(void)
{
    ibv_destroy_cq(m_cq);
}

uint32_t IbCompQueue::PollForCompletion(bool blocking, uint64_t* workReqId, uint32_t* recvLength)
{
    struct ibv_wc wc;
    int ret;

    if (recvLength != nullptr) {
        *recvLength = 0;
    }

    // poll completion queue until one is available or error
    if (blocking) {
        do {
            ret = ibv_poll_cq(m_cq, 1, &wc);
        } while (ret == 0);
    } else {
        ret = ibv_poll_cq(m_cq, 1, &wc);

        // queue empty
        if (ret == 0) {
            return (uint32_t) -1;
        }
    }

    // error polling cq
    if (ret < 0) {
        throw IbException("Polling completion queue failed: " +
                std::to_string(ret));
    }

    // polling successful, check work completion status
    if (wc.status != IBV_WC_SUCCESS) {
        if (wc.status) {
            switch (wc.status) {
                // a previous work request failed and put the queue into error
                // state
//                case IBV_WC_WR_FLUSH_ERR:
//                    return (uint32_t) -1;

                case IBV_WC_RETRY_EXC_ERR:
                    if (m_firstWc) {
                        throw IbException("First work completion of queue "
                            "failed, it's very likely your connection "
                            "attributes are wrong or the remote site isn't in "
                            "a state to respond");
                    } else {
                        throw IbDisconnectedException();
                    }

                default:
                    throw IbException("Found failed work completion, status " +
                                      std::to_string(wc.status));
            }
        }
    }

    if (workReqId != nullptr) {
        *workReqId = wc.wr_id;
    }

    if (recvLength != nullptr) {
        *recvLength = wc.byte_len;
    }

    m_firstWc = false;

    if (!m_outstandingComps.SubOutstanding()) {
        throw IbException("Outstanding queue underrun");
    }

    return wc.qp_num;
}

uint32_t IbCompQueue::Flush(void)
{
    uint32_t count = 0;

    // poll off outstanding completions

    while (m_outstandingComps.GetCurrent() > 0) {
        PollForCompletion(true);
        count++;
    }

    return count;
}

}
}