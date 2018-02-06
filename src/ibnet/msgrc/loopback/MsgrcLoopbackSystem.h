//
// Created by nothaas on 1/31/18.
//

#ifndef IBNET_MSGRC_MSGRCLOOPBACKSYSTEM_H
#define IBNET_MSGRC_MSGRCLOOPBACKSYSTEM_H

#include "ibnet/msgrc/MsgrcSystem.h"

namespace ibnet {
namespace msgrc {

class MsgrcLoopbackSystem : public MsgrcSystem
{
public:
    MsgrcLoopbackSystem(int argc, char** argv);

    ~MsgrcLoopbackSystem() override = default;

    void NodeDiscovered(con::NodeId nodeId) override;

    void NodeInvalidated(con::NodeId nodeId) override;

    void NodeConnected(con::Connection& connection) override;

    void NodeDisconnected(con::NodeId nodeId) override;

    void Received(ReceivedPackage* recvPackage) override;

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
