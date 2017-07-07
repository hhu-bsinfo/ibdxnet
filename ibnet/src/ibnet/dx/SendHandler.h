#ifndef IBNET_DX_SENDHANDLER_H
#define IBNET_DX_SENDHANDLER_H

#include "JNIHelper.h"

namespace ibnet {
namespace dx {

class SendHandler
{
public:
    struct NextWorkParameters
    {
        uint64_t m_ptrBuffer;
        uint32_t m_len;
        uint32_t m_flowControlData;
        uint16_t m_nodeId;
    } __attribute__((packed));

public:
    SendHandler(JNIEnv* env, jobject object);
    ~SendHandler(void);

    inline NextWorkParameters* GetNextDataToSend(uint16_t prevNodeIdWritten,
            uint32_t prevDataWrittenLen, uint64_t prevFlowControlWritten)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);
        jlong ret = env->CallLongMethod(m_object, m_midGetNextDataToSend,
            prevNodeIdWritten, prevDataWrittenLen, prevFlowControlWritten);
        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;

        return (NextWorkParameters*) ret;
    }

private:
    JavaVM* m_vm;
    jobject m_object;

    jmethodID m_midGetNextDataToSend;
};

}
}

#endif //IBNET_DX_SENDHANDLER_H
