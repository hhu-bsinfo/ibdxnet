//
// Created by nothaas on 1/29/18.
//

#ifndef IBNET_CON_INVALIDNODEIDEXCEPTION_H
#define IBNET_CON_INVALIDNODEIDEXCEPTION_H

#include "ibnet/sys/Exception.h"

namespace ibnet {
namespace con {

/**
 * Exception thrown if the specified node id (e.g. parameter) was invalid
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class InvalidNodeIdException : public sys::Exception
{
public:
    /**
     * Constructor
     */
    InvalidNodeIdException() :
        Exception("The specified node id is invalid")
    {}

    /**
     * Constructor
     *
     * @param nodeId The node id considered not valid in this case
     * @param format Printf style format message
     * @param args Parameters for format string
     */
    template<typename... Args>
    InvalidNodeIdException(con::NodeId nodeId, const std::string& format,
            Args... args) :
        Exception("The specified node id %X is not valid, reason: " + format,
            nodeId, args...)
    {}

    /**
     * Destructor
     */
    ~InvalidNodeIdException() override = default;
};

}
}

#endif //IBNET_CON_INVALIDNODEIDEXCEPTION_H
