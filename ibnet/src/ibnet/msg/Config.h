#ifndef IBNET_MSG_CONFIG_H
#define IBNET_MSG_CONFIG_H

namespace ibnet {
namespace msg {

struct Config
{
    uint16_t m_maxRecvReqs;
    uint16_t m_maxSendReqs;
    uint32_t m_inOutBufferSize;

    uint16_t m_flowControlMaxRecvReqs;
    uint16_t m_flowControlMaxSendReqs;

    uint32_t m_connectionJobPoolSize;
    uint8_t m_sendThreads;
    uint8_t m_recvThreads;

    uint16_t m_maxNumConnections;

    Config(void) :
        m_maxRecvReqs(10),
        m_maxSendReqs(10),
        m_inOutBufferSize(8192),
        m_flowControlMaxRecvReqs(10),
        m_flowControlMaxSendReqs(10),
        m_connectionJobPoolSize(1000),
        m_sendThreads(1),
        m_recvThreads(1),
        m_maxNumConnections(100)
    {};

    ~Config(void) {};
};

}
}

#endif //IBNET_MSG_CONFIG_H
