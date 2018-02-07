//
// Created by on 1/31/18.
//

#include "MsgrcJNISystem.h"

#include "ibnet/sys/TimeoutException.h"

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

uint32_t MsgrcJNISystem::CreateConnection(con::NodeId nodeId)
{
    // force connection creation
    try {
        Connection* con = dynamic_cast<Connection*>(
            m_connectionManager->GetConnection(nodeId));
        m_connectionManager->ReturnConnection(con);
    } catch (sys::TimeoutException& e) {
        IBNET_LOG_WARN("CreateConnection (0x%X): %s", nodeId, e.what());
        return 1;
    } catch (sys::Exception& e) {
        IBNET_LOG_WARN("CreateConnection (0x%X): %s", nodeId, e.what());
        return 2;
    }

    IBNET_LOG_DEBUG("Created connection to 0x%X", nodeId);

    return 0;
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
        IBNET_LOG_ERROR("Getting send buffer address for 0x%X failed, "
            "connection not created, yet", targetNodeId);
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
    // unused
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