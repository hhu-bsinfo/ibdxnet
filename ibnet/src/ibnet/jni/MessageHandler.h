#ifndef IBNET_JNI_MESSAGEHANDLER_H
#define IBNET_JNI_MESSAGEHANDLER_H

#include "Callbacks.h"

namespace ibnet {
namespace jni {

class MessageHandler : public ibnet::msg::MessageHandler
{
public:
    MessageHandler(std::shared_ptr<Callbacks> callbacks) :
        m_callbacks(callbacks)
    {
    };

    ~MessageHandler(void)
    {
    };

    void PrintTimers(void)
    {
        // TODO
    }

    inline void HandleMessage(uint16_t source, void* buffer, uint32_t length)
        override
    {
        m_callbacks->HandleReceive(source, buffer, length);
    }

    inline void HandleFlowControlData(uint16_t source, uint32_t data) override
    {
        m_callbacks->HandleReceiveFlowControlData(source, data);
    }

private:
    std::shared_ptr<Callbacks> m_callbacks;

    ibnet::sys::ProfileTimer receivedBufferTimer;
};

}
}

#endif //IBNET_JNI_MESSAGEHANDLER_H
