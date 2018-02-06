//
// Created by nothaas on 1/30/18.
//

#ifndef IBNET_DX_EXECUTIONUNIT_H
#define IBNET_DX_EXECUTIONUNIT_H

#include <string>

namespace ibnet {
namespace dx {

class ExecutionUnit
{
public:
    const std::string& GetName() const {
        return m_name;
    }

    virtual bool Dispatch() = 0;

protected:
    explicit ExecutionUnit(const std::string& name) :
        m_name(name)
    {}

    virtual ~ExecutionUnit() = default;

private:
    const std::string m_name;
};

}
}

#endif //IBNET_DX_EXECUTIONUNIT_H
