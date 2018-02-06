//
// Created by on 1/31/18.
//

#include "MsgrcJNISystem.h"

namespace ibnet {
namespace msgrc {

MsgrcJNISystem::MsgrcJNISystem(MsgrcSystem::Configuration* configuration,
        JNIEnv* env, jobject callbackHandler) :
    MsgrcSystem(),
    m_callbackHandler(env, callbackHandler),
    m_workPackage()
{
    _SetConfiguration(configuration);
}

void MsgrcJNISystem::AddNode(uint32_t ipv4)
{
    IBNET_LOG_TRACE_FUNC;

    ibnet::con::NodeConf::Entry entry(ibnet::sys::AddressIPV4((uint32_t) ipv4));
    m_discoveryManager->AddNode(entry);
}

core::IbMemReg* MsgrcJNISystem::GetSendBuffer(con::NodeId targetNodeId)
{
    Connection* connection = nullptr;

    try {
        connection = (Connection*)
            m_connectionManager->GetConnection(targetNodeId);
    } catch (sys::Exception& e) {
        IBNET_LOG_ERROR("%s", e.what());
        return nullptr;
    }

    if (!connection) {
        return nullptr;
    }

    ibnet::core::IbMemReg* refBuffer = connection->GetRefSendBuffer();

    m_connectionManager->ReturnConnection(connection);

    return refBuffer;
}

void MsgrcJNISystem::ReturnRecvBuffer(core::IbMemReg* buffer)
{
    IBNET_LOG_TRACE("Return recv buffer handle %p, buffer addr %p, size %d",
        (uintptr_t) buffer, (uintptr_t) buffer->GetAddress(),
        buffer->GetSize());

    m_recvBufferPool->ReturnBuffer(buffer);
}

void MsgrcJNISystem::NodeDiscovered(con::NodeId nodeId)
{
    m_callbackHandler.NodeDiscovered(nodeId);
}

void MsgrcJNISystem::NodeInvalidated(con::NodeId nodeId)
{
    m_callbackHandler.NodeInvalidated(nodeId);
}

void MsgrcJNISystem::NodeConnected(con::Connection& connection)
{
    m_callbackHandler.NodeConnected(connection.GetRemoteNodeId());
}

void MsgrcJNISystem::NodeDisconnected(con::NodeId nodeId)
{
    m_callbackHandler.NodeDisconnected(nodeId);
}

void MsgrcJNISystem::Received(ReceivedPackage* recvPackage)
{
    m_callbackHandler.Received(recvPackage);
}

const SendHandler::NextWorkPackage* MsgrcJNISystem::GetNextDataToSend(
        const SendHandler::PrevWorkPackageResults* prevResults,
        const SendHandler::CompletedWorkList* completionList)
{
    return m_callbackHandler.GetNextDataToSend(prevResults, completionList);
}

}
}