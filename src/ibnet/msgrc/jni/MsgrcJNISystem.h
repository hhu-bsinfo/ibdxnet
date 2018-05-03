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

/**
 * System for hooking the RC messaging system to Java bindings.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 31.01.2018
 */
class MsgrcJNISystem : public MsgrcSystem
{
public:
    /**
     * Constructor
     *
     * @param configuration Configuration to set for MsgrcSystem (memory managed by system)
     * @param env Pointer to the JNI environment
     * @param callbackHandler Pointer to the jobject callback handler
     */
    MsgrcJNISystem(MsgrcSystem::Configuration* configuration, JNIEnv* env,
            jobject callbackHandler);

    /**
     * Destructor
     */
    ~MsgrcJNISystem() override = default;

    /**
     * Add a new node to the system (for discovery and connection management)
     *
     * @param ipv4 IPV4 address of the target node
     */
    void AddNode(uint32_t ipv4);

    /**
     * Actively create a connection to a target node
     *
     * @param nodeId Target node id to create a connection to
     * @return 0 for success, 1 for creation timeout, 2 for any other error
     */
    uint32_t CreateConnection(con::NodeId nodeId);

    /**
     * Get the memory region of the send buffer of a target connection
     *
     * @param targetNodeId Node id of the target connection
     * @return Pointer to the memory region of the send buffer or nullptr
     *         if the connection does not exist, yet.
     */
    core::IbMemReg* GetSendBuffer(con::NodeId targetNodeId);

    /**
     * Return a receive buffer to the pool
     *
     * @param buffer Pointer to the buffer to return
     */
    void ReturnRecvBuffer(core::IbMemReg* buffer);

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
            const SendHandler::PrevWorkPackageResults* prevResults,
            const SendHandler::CompletedWorkList* completionList) override;

private:
    MsgrcJNIBindingCallbackHandler m_callbackHandler;
    SendHandler::NextWorkPackage m_workPackage;
};

}
}

#endif //IBNET_MSGRC_MSGRCJNISYSTEM_H
