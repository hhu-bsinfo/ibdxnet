#ifndef IBNET_MSG_CONFIG_H
#define IBNET_MSG_CONFIG_H

namespace ibnet {
namespace msg {

/**
 * Config for IbMessageSystem
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
struct Config
{
    /**
     * Receive queue size for buffer data
     */
    uint16_t m_maxRecvReqs;

    /**
     * Send queue size for buffer data
     */
    uint16_t m_maxSendReqs;

    /**
     * Max size per buffer (in and out)
     */
    uint32_t m_inOutBufferSize;

    /**
     * Receive queue size for flow control data
     */
    uint16_t m_flowControlMaxRecvReqs;

    /**
     * Send queue size for flow control data
     */
    uint16_t m_flowControlMaxSendReqs;

    /**
     * (Send) job pool size per connection
     */
    uint32_t m_connectionJobPoolSize;

    /**
     * Number of send threads
     */
    uint8_t m_sendThreads;

    /**
     * Number of receive threads
     */
    uint8_t m_recvThreads;

    /**
     * Max number of open connections
     */
    uint16_t m_maxNumConnections;

    /**
     * Max number of messages to send or -1 for unlimited
     */
    uint32_t m_maxMessages;

    /**
     * Constructor
     *
     * Sets default values only
     */
    Config(void) :
        m_maxRecvReqs(10),
        m_maxSendReqs(10),
        m_inOutBufferSize(8192),
        m_flowControlMaxRecvReqs(10),
        m_flowControlMaxSendReqs(10),
        m_connectionJobPoolSize(1000),
        m_sendThreads(1),
        m_recvThreads(1),
        m_maxNumConnections(100),
        m_maxMessages(-1)
    {};

    ~Config(void) {};
};

}
}

#endif //IBNET_MSG_CONFIG_H
