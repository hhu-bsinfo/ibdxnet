#ifndef IBNET_CORE_IBPROTDOM_H
#define IBNET_CORE_IBPROTDOM_H

#include <memory>

#include "IbDevice.h"
#include "IbMemReg.h"

namespace ibnet {
namespace core {

class IbProtDom
{
public:
    IbProtDom(std::shared_ptr<IbDevice>& device, const std::string& name);
    ~IbProtDom(void);

    std::shared_ptr<IbMemReg> Register(void* addr, uint32_t size, bool freeOnCleanup = true);

    ibv_pd* GetIBProtDom(void) const {
        return m_ibProtDom;
    }

    friend std::ostream &operator<<(std::ostream& os, const IbProtDom& o) {
        os << o.m_name << ":";

        for (auto& it : o.m_registeredRegions) {
            os << "\n" << it;
        }

        return os;
    }

private:
    const std::string m_name;
    ibv_pd* m_ibProtDom;

    std::vector<std::shared_ptr<IbMemReg>> m_registeredRegions;
};

}
}

#endif // IBNET_CORE_IBPROTDOM_H
