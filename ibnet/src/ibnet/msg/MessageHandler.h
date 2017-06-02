#ifndef IBNET_MSG_MESSAGEHANDLER_H
#define IBNET_MSG_MESSAGEHANDLER_H

namespace ibnet {
namespace msg {

/**
 * Interface to handle incoming buffers and flow control
 * data. Register your message handler on construction
 * of the IbMessageSystem to receive incoming data
 * from other nodes
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class MessageHandler
{
public:
    /**
     * Constructor
     */
    MessageHandler(void) {};

    /**
     * Destructor
     */
    virtual ~MessageHandler(void) {};

    /**
     * Handle an incoming message/buffer
     *
     * Note: The handler has to copy the data from the buffer to his
     * own buffer because the provided buffer is owned by the receive
     * thread and can not be shared
     *
     * @param source Source node id of the buffer
     * @param buffer Pointer to allocated buffer with data
     * @param length Length of data (bytes)
     */
    virtual void HandleMessage(uint16_t source, void* buffer, uint32_t length) = 0;

    /**
     * Handle incoming flow control data
     *
     * @param source Source node id of the data
     * @param data Flow control data
     */
    virtual void HandleFlowControlData(uint16_t source, uint32_t data) = 0;
};

}
}

#endif //IBNET_MSG_MESSAGEHANDLER_H
