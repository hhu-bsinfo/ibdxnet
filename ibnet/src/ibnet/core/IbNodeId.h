#ifndef IBNET_CORE_IBNODEID_H
#define IBNET_CORE_IBNODEID_H

namespace ibnet {
namespace core {

/**
 * Abstract node id to identify nodes in an InfiniBand network
 * (not part of ibverbs or InfiniBand hardware)
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbNodeId
{
public:
    static const uint16_t INVALID = 0xFFFF;
    static const uint16_t MAX_NUM_NODES = 0xFFFF;
};

}
}

#endif // IBNET_CORE_IBNODEID_H
