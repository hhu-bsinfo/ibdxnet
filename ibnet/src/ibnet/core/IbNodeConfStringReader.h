#ifndef IBNET_CORE_IBNODECONFSTRINGREADER_H
#define IBNET_CORE_IBNODECONFSTRINGREADER_H

#include "IbNodeConfReader.h"

namespace ibnet {
namespace core {

class IbNodeConfStringReader : public IbNodeConfReader
{
public:
    IbNodeConfStringReader(const std::string& str);
    ~IbNodeConfStringReader(void);

    IbNodeConf Read(void) override;

private:
    const std::string& m_str;
};

}
}


#endif // IBNET_CORE_IBNODECONFSTRINGREADER_H
