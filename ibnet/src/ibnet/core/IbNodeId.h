#ifndef IBNET_CORE_IBNODEID_H
#define IBNET_CORE_IBNODEID_H

namespace ibnet {
namespace core {

class IbNodeId
{
public:
    static const uint16_t INVALID = 0xFFFF;
    static const uint16_t MAX_NUM_NODES = 0xFFFF;
};

}
}

#endif // IBNET_CORE_IBNODEID_H
