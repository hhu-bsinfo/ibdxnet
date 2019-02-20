#ifndef IBNET_MSGUD_CCOMMON_H
#define IBNET_MSGUD_CCOMMON_H

#include <cstdint>

#include "ibnet/con/NodeId.h"

namespace ibnet {
namespace msgud {

/**
 * Structure for accessing data stored in the immediate data field.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 29.01.2018
 * @author Fabian Ruhland, fabian.ruhland@hhu.de, 12.04.2018
 */
struct ImmediateData
{
    con::NodeId m_sourceNodeId;
    uint8_t m_flowControlData;
    uint8_t m_endOfWorkPackage : 1;
    uint8_t m_sequenceNumber : 7;

    /**
     * Overloading << operator for printing to ostreams
     *
     * @param os Ostream to output to
     * @param o Operation to generate output for
     * @return Ostream object
     */
    friend std::ostream& operator<<(std::ostream& os, const ImmediateData& o)
    {
        return os << "m_sourceNodeId " << std::hex << o.m_sourceNodeId << std::dec << ", m_flowControlData " <<
                  static_cast<uint16_t>(o.m_flowControlData) << ", m_endOfWorkPackage " <<
                  static_cast<uint16_t>(o.m_endOfWorkPackage) << ", m_sequenceNumber " <<
                  static_cast<uint16_t>(o.m_sequenceNumber);
    }
} __attribute__((__packed__));

/**
 * Structure wrapping a simple counter.
 * Used for sequence numbers.
 *
 * @author Fabian Ruhland, fabian.ruhland@hhu.de 18.03.2018
 */
struct Counter
{
public:
    inline uint16_t GetValue() {
        return m_value;
    }

    inline void SetValue(uint16_t value) {
        m_value = value;
    }

    inline void Inc() {
        m_value++;
    }

    inline void Dec() {
        m_value--;
    }

    inline void Reset() {
        m_value = 0;
    }

private:
    uint16_t m_value = 0;
};

/**
 * Enum for holding special sequence number values.
 *
 * @author Fabian Ruhland, 20.02.2019
 */
enum AckSequenceNumber : uint8_t
{
    SEQUENCE_NUMBER_ACK = 126,
    SEQUENCE_NUMBER_NACK = 127
};

/**
 * Enum for signalizing the status of an ACK.
 *
 * @author Fabian Ruhland, 20.02.2019
 */
enum AckStatus
{
    IDLE, /** Signalizing, that the connection is currently not waiting for an ACK */
    WAITING, /** Signalizing, that the connection is waiting for the next ACK */
    ACK_RECEIVED, /**< Signalizing successful transfer of the last work package */
    NACK_RECEIVED /**< Signalizing an error (e.g. lost packets) during the transfer of the last work package */
};

}
}

#endif //IBNET_MSGUD_CCOMMON_H
