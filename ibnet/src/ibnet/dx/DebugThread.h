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
        const std::vector<std::unique_ptr<RecvThread>>& recvThreads,
        const std::vector<std::unique_ptr<SendThread>>& sendThreads);

    /**
     * Destructor
     */
    ~DebugThread(void);

protected:
    void _RunLoop(void) override;

private:
    const std::vector<std::unique_ptr<RecvThread>>& m_recvThreads;
    const std::vector<std::unique_ptr<SendThread>>& m_sendThreads;
};

}
}

#endif //IBNET_DX_DEBUGTHREAD_H
