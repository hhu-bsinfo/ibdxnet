#ifndef IBNET_DX_DXNETEXCEPTION_H
#define IBNET_DX_DXNETEXCEPTION_H

#include "ibnet/sys/Exception.h"

namespace ibnet {
namespace dx {

/**
 * Common exception for the dxnet namespace
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class DxnetException : public sys::Exception
{
public:
    /**
     * Constructor
     *
     * @param msg Exception message
     */
    DxnetException(const std::string& msg) :
            Exception(msg)
    {

    }

    /**
     * Destructor
     */
    ~DxnetException(void) {};
};

}
}

#endif // IBNET_DX_DXNETEXCEPTION_H