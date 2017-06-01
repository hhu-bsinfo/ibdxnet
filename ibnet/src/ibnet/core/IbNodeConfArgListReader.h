#ifndef IBNET_CORE_IBNODECONFARGLISTREADER_H
#define IBNET_CORE_IBNODECONFARGLISTREADER_H

#include "IbNodeConfReader.h"

namespace ibnet {
namespace core {

class IbNodeConfArgListReader : public IbNodeConfReader
{
public:
    IbNodeConfArgListReader(uint32_t numItems, char** args);
    ~IbNodeConfArgListReader(void);

    IbNodeConf Read(void) override;

private:
    uint32_t m_numItems;
    char** m_args;
};

}
}

#endif // IBNET_CORE_IBNODECONFARGLISTREADER_H
