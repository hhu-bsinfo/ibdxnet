#ifndef IBNET_CORE_IBCONNECTIONCREATOR_H
#define IBNET_CORE_IBCONNECTIONCREATOR_H

#include "IbDevice.h"
#include "IbConnection.h"
#include "IbProtDom.h"

namespace ibnet {
namespace core {

/**
 * Interface for a connection creator telling the connection manager how
 * to setup a newly established connection.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbConnectionCreator
{
public:
    /**
     * Constructor
     */
    IbConnectionCreator(void) {};

    /**
     * Destructor
     */
    virtual ~IbConnectionCreator(void) {};

    /**
     * Create a new connection
     *
     * @param connectionId Id for the new connection
     * @param device Device
     * @param protDom Protection domain
     * @return Instance of a new connection
     */
    virtual std::shared_ptr<IbConnection> CreateConnection(
        uint16_t connectionId,
        std::shared_ptr<IbDevice>& device,
        std::shared_ptr<IbProtDom>& protDom) = 0;
};

}
}

#endif //IBNET_CORE_IBCONNECTIONCREATOR_H
