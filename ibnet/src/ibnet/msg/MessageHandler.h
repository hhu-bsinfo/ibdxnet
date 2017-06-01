#ifndef IBNET_MSG_MESSAGEHANDLER_H
#define IBNET_MSG_MESSAGEHANDLER_H

namespace ibnet {
namespace msg {

class MessageHandler
{
public:
    MessageHandler(void) {};
    virtual ~MessageHandler(void) {};

    // note: Message handler has to copy the data from the buffer to his own
    // buffer is owned and managed by the receive thread(s)
    virtual void HandleMessage(uint16_t source, void* buffer, uint32_t length) = 0;

    virtual void HandleFlowControlData(uint16_t source, uint32_t data) = 0;
};

}
}

#endif //IBNET_MSG_MESSAGEHANDLER_H
