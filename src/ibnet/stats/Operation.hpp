//
// Created by nothaas on 2/1/18.
//

#ifndef IBNET_STATS_OPERATION_HPP
#define IBNET_STATS_OPERATION_HPP

#include <ostream>
#include <string>

namespace ibnet {
namespace stats {

class Operation
{
public:
    explicit Operation(const std::string& name) :
        m_name(name)
    {}

    virtual ~Operation() = default;

    const std::string& GetName() const {
        return m_name;
    }

    virtual void WriteOstream(std::ostream& os) const = 0;

    friend std::ostream &operator<<(std::ostream& os, const Operation& o) {
        os << '[' << o.m_name << "] ";
        o.WriteOstream(os);
        return os;
    }

private:
    const std::string m_name;
};

}
}

#endif //IBNET_STATS_OPERATION_HPP
