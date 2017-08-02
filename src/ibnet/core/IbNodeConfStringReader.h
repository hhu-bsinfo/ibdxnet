#ifndef IBNET_CORE_IBNODECONFSTRINGREADER_H
#define IBNET_CORE_IBNODECONFSTRINGREADER_H

#include "IbNodeConfReader.h"

namespace ibnet {
namespace core {

/**
 * Implementation of a IbNodeConfReader reading a nodes configuration from
 * a string
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbNodeConfStringReader : public IbNodeConfReader
{
public:
    /**
     * Constructor
     *
     * @param str String containing the nodes configuration,
     *          e.g. "node65 node66"
     */
    IbNodeConfStringReader(const std::string& str);

    /**
     * Destructor
     */
    ~IbNodeConfStringReader(void);

    IbNodeConf Read(void) override;

private:
    const std::string& m_str;
};

}
}


#endif // IBNET_CORE_IBNODECONFSTRINGREADER_H
