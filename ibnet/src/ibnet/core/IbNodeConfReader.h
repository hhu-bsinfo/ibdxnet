#ifndef IBNET_CORE_IBNODECONFREADER_H
#define IBNET_CORE_IBNODECONFREADER_H

#include "IbNodeConf.h"

namespace ibnet {
namespace core {

/**
 * Interface for a node configuration reader to get a list of nodes forming
 * an InfiniBand network
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbNodeConfReader
{
public:
    /**
     * Constructor
     */
    IbNodeConfReader(void) {};

    /**
     * Desturctor
     */
    virtual ~IbNodeConfReader(void) {};

    /**
     * Read the configuration
     *
     * @return IbNodeConf object with nodes of the network
     */
    virtual IbNodeConf Read(void) = 0;
};

}
}

#endif // IBNET_CORE_IBNODECONFREADER_H
