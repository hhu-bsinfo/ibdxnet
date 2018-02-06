//
// Created by nothaas on 1/30/18.
//

#ifndef IBNET_MSGRC_CCOMMON_H
#define IBNET_MSGRC_CCOMMON_H

#include <cstdint>

#include "ibnet/con/NodeId.h"

namespace ibnet {
namespace msgrc {

struct ImmediateData
{
    con::NodeId m_sourceNodeId;
    uint8_t m_zeroLengthData : 1;
    uint8_t m_flowControlData : 7;
    uint8_t m_dummy;
} __attribute__((__packed__));

}
}

#endif //IBNET_MSGRC_CCOMMON_H
