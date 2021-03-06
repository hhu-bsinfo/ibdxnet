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

#ifndef IBNET_MSGRC_SENDHANDLER_H
#define IBNET_MSGRC_SENDHANDLER_H

#include <cstdint>
#include <cstring>

#include "ibnet/con/NodeId.h"

namespace ibnet {
namespace msgrc {

/**
 * The SendDispatcher is calling the SendHandler, asking for new data to send.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 31.01.2018
 */
class SendHandler
{
public:
    /**
     * A work request package that defines which data to be sent next.
     * Returned using a pointer by the callback
     */
    struct NextWorkPackage
    {
        uint32_t m_posBackRel;
        uint32_t m_posFrontRel;
        uint8_t m_flowControlData;
        con::NodeId m_nodeId;

        friend std::ostream& operator<<(std::ostream& os,
                const NextWorkPackage& o)
        {
            return os << "m_posBackRel " << o.m_posBackRel << ", m_posFrontRel "
                    << o.m_posFrontRel << ", m_flowControlData " <<
                    o.m_flowControlData << ", m_nodeId " << std::hex << o.m_nodeId;
        }
    } __attribute__((packed));

    /**
     * Data about the previous work package processed: how much data was processed
     * and if the queue was full, how much data could not be sent out. If the data
     * was processed, it is not guaranteed that it was actually sent. You have to
     * wait for a work completion to actually know it was sent (see CompletedWorkList)
     */
    struct PrevWorkPackageResults
    {
        con::NodeId m_nodeId;
        uint32_t m_numBytesPosted;
        uint32_t m_numBytesNotPosted;
        uint8_t m_fcDataPosted;
        uint8_t m_fcDataNotPosted;

        PrevWorkPackageResults() :
                m_nodeId(con::NODE_ID_INVALID),
                m_numBytesPosted(0),
                m_numBytesNotPosted(0),
                m_fcDataPosted(0),
                m_fcDataNotPosted(0)
        {
        }

        void Reset()
        {
            m_nodeId = con::NODE_ID_INVALID;
            m_numBytesPosted = 0;
            m_numBytesNotPosted = 0;
            m_fcDataPosted = 0;
            m_fcDataNotPosted = 0;
        }

        friend std::ostream& operator<<(std::ostream& os,
                const PrevWorkPackageResults& o)
        {
            return os << "m_nodeId " << std::hex << o.m_nodeId <<
                    ", m_numBytesPosted " << std::dec << o.m_numBytesPosted <<
                    ", m_numBytesNotPosted " << o.m_numBytesNotPosted <<
                    ", m_fcDataPosted " << static_cast<uint16_t>(o.m_fcDataPosted)
                    << ", m_fcDataNotPosted " <<
                    static_cast<uint16_t>(o.m_fcDataNotPosted);
        }
    } __attribute__((packed));

    /**
     * List of work completions that arrived confirming that data was
     * actually sent.
     */
    struct CompletedWorkList
    {
        static size_t Sizeof(uint32_t numNodes)
        {
            return sizeof(uint16_t) +
                    sizeof(uint32_t) * con::NODE_ID_MAX_NUM_NODES +
                    sizeof(uint8_t) * con::NODE_ID_MAX_NUM_NODES +
                    numNodes * sizeof(con::NodeId);
        }

        uint16_t m_numNodes;
        uint32_t m_numBytesWritten[con::NODE_ID_MAX_NUM_NODES];
        uint8_t m_fcDataWritten[con::NODE_ID_MAX_NUM_NODES];
        con::NodeId m_nodeIds[];

        void Reset()
        {
            for (uint16_t i = 0; i < m_numNodes; i++) {
                m_numBytesWritten[m_nodeIds[i]] = 0;
                m_fcDataWritten[m_nodeIds[i]] = 0;
                m_nodeIds[i] = con::NODE_ID_INVALID;
            }

            m_numNodes = 0;
        }

        friend std::ostream& operator<<(std::ostream& os,
                const CompletedWorkList& o)
        {
            os << "m_numNodes " << o.m_numNodes << ", m_entries";

            for (uint16_t i = 0; i < o.m_numNodes; i++) {
                os << " " << std::hex << o.m_nodeIds[i] << "|" <<
                        std::dec << o.m_numBytesWritten[o.m_nodeIds[i]] << "|" <<
                        static_cast<uint16_t>(o.m_fcDataWritten[o.m_nodeIds[i]]);
            }

            return os;
        }
    } __attribute__((packed));

public:
    /**
     * Called by the SendDispatcher asking for more data to send
     *
     * @param prevResults Pointer to data with information about the previous processed
     *        data (caller is managing memory)
     * @param completionList Pointer to data which informs about completed work requests
     *        i.e. sending of data confirmed (caller is managing memory). You have to
     *        empty this list if it contains any completions
     * @return Return a pointer to a NextWorkPackage structure which includes the data
     *         to be sent. The caller is not managing memory.
     */
    virtual const NextWorkPackage* GetNextDataToSend(
            const PrevWorkPackageResults* prevResults,
            const CompletedWorkList* completionList) = 0;

protected:
    /**
     * Constructor
     */
    SendHandler() = default;

    /**
     * Destructor
     */
    virtual ~SendHandler() = default;
};

}
}

#endif //IBNET_MSGRC_SENDHANDLER_H
