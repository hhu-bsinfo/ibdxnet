#ifndef IBNET_CORE_IBNODECONFARGLISTREADER_H
#define IBNET_CORE_IBNODECONFARGLISTREADER_H

#include "IbNodeConfReader.h"

namespace ibnet {
namespace core {

/**
 * Implementation of a IbNodeConfReader to read a node config from an
 * argument list (main entry point).
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbNodeConfArgListReader : public IbNodeConfReader
{
public:
    /**
     * Constructor
     *
     * @param numItems Number of elements from cmd arguments
     * @param args Arguments
     */
    IbNodeConfArgListReader(uint32_t numItems, char** args);

    /**
     * Destructor
     */
    ~IbNodeConfArgListReader(void);

    IbNodeConf Read(void) override;

private:
    uint32_t m_numItems;
    char** m_args;
};

}
}

#endif // IBNET_CORE_IBNODECONFARGLISTREADER_H
