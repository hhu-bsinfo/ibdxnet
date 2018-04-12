//
// Created by nothaas on 1/30/18.
//

#ifndef IBNET_MSGRC_CCOMMON_H
#define IBNET_MSGRC_CCOMMON_H

#include <cstdint>

#include "ibnet/con/NodeId.h"

namespace ibnet {
namespace msgud {

struct ImmediateData
{
    con::NodeId m_sourceNodeId;
    uint8_t m_zeroLengthData : 1;
    uint8_t m_flowControlData : 7;
    uint8_t m_endOfWorkPackage : 1;
    uint8_t m_sequenceNumber : 7;
} __attribute__((__packed__));

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

}
}

#endif //IBNET_MSGRC_CCOMMON_H
