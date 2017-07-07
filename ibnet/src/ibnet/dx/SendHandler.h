#ifndef IBNET_DX_SENDHANDLER_H
#define IBNET_DX_SENDHANDLER_H

#include "JNIHelper.h"

namespace ibnet {
namespace dx {

/**
 * Provide access to buffers which are available in the jvm space. This
 * is called by the SendThread to get new/next buffers to send
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 07.07.2017
 */
class SendHandler
{
public:
    /**
     * A work request package that defines which data to be sent next
     */
    struct NextWorkParameters
    {
        uint64_t m_ptrBuffer;
        uint32_t m_len;
        uint32_t m_flowControlData;
        uint16_t m_nodeId;
    } __attribute__((packed));

public:
    /**
     * Constructor
     *
     * @param env The java environment from a java thread
     * @param object Java object of the equivalent callback class in java
     */
    SendHandler(JNIEnv* env, jobject object);

    /**
     * Destructor
     */
    ~SendHandler(void);

    /**
     * Called by an instance of SendThread. Get the next buffer/work package
     * to send
     *
     * @param prevNodeIdWritten Node id of the previous next call
     *        (or -1 if there is no previous valid work request)
     * @param prevDataWrittenLen Number of bytes written on the previous work
     *          request
     * @param prevFlowControlWritten Flow control data written on the previous
     *          work request
     * @return Pointer to the next work request package (don't free)
     */
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
