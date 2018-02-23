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

#ifndef IBNET_MSGRC_MSGRCJNISYSTEM_H
#define IBNET_MSGRC_MSGRCJNISYSTEM_H

#include <jni.h>

#include "ibnet/msgrc/MsgrcSystem.h"

#include "MsgrcJNIBindingCallbackHandler.h"

namespace ibnet {
namespace msgrc {

//
// Created by on 1/31/18.
//
class MsgrcJNISystem : public MsgrcSystem
{
public:
    MsgrcJNISystem(MsgrcSystem::Configuration* configuration, JNIEnv* env,
        jobject callbackHandler);

    ~MsgrcJNISystem() override = default;

    void AddNode(uint32_t ipv4);

    // returns 0 for success, 1 for creation timeout, 2 for any other error
    uint32_t CreateConnection(con::NodeId nodeId);

    core::IbMemReg* GetSendBuffer(con::NodeId targetNodeId);

    void ReturnRecvBuffer(core::IbMemReg* buffer);

    void NodeDiscovered(con::NodeId nodeId) override;

    void NodeInvalidated(con::NodeId nodeId) override;

    void NodeConnected(con::Connection& connection) override;

    void NodeDisconnected(con::NodeId nodeId) override;

    void Received(ReceivedPackage* recvPackage) override;

    const SendHandler::NextWorkPackage* GetNextDataToSend(
        const SendHandler::PrevWorkPackageResults* prevResults,
        const SendHandler::CompletedWorkList* completionList) override;

private:
    MsgrcJNIBindingCallbackHandler m_callbackHandler;
    SendHandler::NextWorkPackage m_workPackage;
};

}
}

#endif //IBNET_MSGRC_MSGRCJNISYSTEM_H
