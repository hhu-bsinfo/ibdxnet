#ifndef IBNET_DX_DEBUGTHREAD_H
#define IBNET_DX_DEBUGTHREAD_H

#include "ibnet/sys/ThreadLoop.h"

#include "RecvThread.h"
#include "SendThread.h"

namespace ibnet {
namespace dx {

/**
 * Print performance and debug data periodically
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 07.07.2017
 */
class DebugThread : public sys::ThreadLoop
{
public:
    /**
     * Constructor
     *
     * @param recvThreads Vector with active receive threads
     * @param sendThreads Vector with active send threads
     */
    DebugThread(
        std::shared_ptr<RecvThread> recvThreads,
        std::shared_ptr<SendThread> sendThreads);

    /**
     * Destructor
     */
    ~DebugThread(void);

protected:
    void _RunLoop(void) override;

private:
    const std::shared_ptr<RecvThread> m_recvThread;
    const std::shared_ptr<SendThread> m_sendThread;
};

}
}

#endif //IBNET_DX_DEBUGTHREAD_H
