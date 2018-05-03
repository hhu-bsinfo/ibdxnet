/*
 * Copyright (C) 2018 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef IBNET_MSGRC_MSGRCLOOPBACKSYSTEM_H
#define IBNET_MSGRC_MSGRCLOOPBACKSYSTEM_H

#include "ibnet/msgrc/MsgrcSystem.h"

namespace ibnet {
namespace msgrc {

/**
 * Loopback test/debugging system for the RC messaging system. This simply
 * sends out the same buffer over and over and discards any received data
 * by putting buffers with received data right back to the buffer pool.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 31.01.2018
 */
class MsgrcLoopbackSystem : public MsgrcSystem
{
public:
    /**
     * Constructor
     *
     * @param argc Argc
     * @param argv Argv
     */
    MsgrcLoopbackSystem(int argc, char** argv);

    /**
     * Destructor
     */
    ~MsgrcLoopbackSystem() override = default;

    /**
     * Overriding virtual function
     */
    void NodeDiscovered(con::NodeId nodeId) override;

    /**
     * Overriding virtual function
     */
    void NodeInvalidated(con::NodeId nodeId) override;

    /**
     * Overriding virtual function
     */
    void NodeConnected(con::Connection& connection) override;

    /**
     * Overriding virtual function
     */
    void NodeDisconnected(con::NodeId nodeId) override;

    /**
     * Overriding virtual function
     */
    uint32_t Received(ReceivedPackage* recvPackage) override;

    /**
     * Overriding virtual function
     */
    const SendHandler::NextWorkPackage* GetNextDataToSend(
            const PrevWorkPackageResults* prevResults,
            const SendHandler::CompletedWorkList* completionList) override;

protected:
    void _PostInit() override;

private:
    Configuration* __ProcessCmdArgs(int argc, char** argv);

private:
    std::vector<con::NodeId> m_sendTargetNodeIds;
    bool m_availableTargetNodes[con::NODE_ID_MAX_NUM_NODES];
    std::atomic<con::NodeId> m_targetNodesAvailable;

    SendHandler::NextWorkPackage m_workPackage;
    con::NodeId m_nodeToSendToPos;
};

}
}

#endif //IBNET_MSGRC_MSGRCLOOPBACKSYSTEM_H
