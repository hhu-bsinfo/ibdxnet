//
// Created by nothaas on 2/5/18.
//

#ifndef IBNET_STATS_RATIO_HPP
#define IBNET_STATS_RATIO_HPP

#include <iomanip>
#include <iostream>

#include "Unit.hpp"

namespace ibnet {
namespace stats {

class Ratio : public Operation
{
public:
    explicit Ratio(const std::string& name) :
        Operation(name),
        m_refs(false),
        m_unit1(new Unit(name + "-Unit1")),
        m_unit2(new Unit(name + "-Unit2"))
    {}

    Ratio(const std::string& name, Unit* refUnit1, Unit* refUnit2) :
        Operation(name),
        m_refs(true),
        m_unit1(refUnit1),
        m_unit2(refUnit2)
    {}

    ~Ratio() override
    {
        if (!m_refs) {
            delete m_unit1;
            delete m_unit2;
        }
    }

    inline double GetRatioCounter() const {
        double tmp = m_unit2->GetCounter();
        return m_unit1->GetCounter() / (tmp == 0.0 ? 1.0 : tmp);
    }

    inline double GetRatioTotalValue() const {
        double tmp = m_unit2->GetTotalValue();
        return m_unit1->GetTotalValue() / (tmp == 0.0 ? 1.0 : tmp);
    }

    inline double GetRatioMinValue() const {
        double tmp = m_unit2->GetMinValue();
        return m_unit1->GetMinValue() / (tmp == 0.0 ? 1.0 : tmp);
    }

    inline double GetRatioMaxValue() const {
        double tmp = m_unit2->GetMaxValue();
        return m_unit1->GetMaxValue() / (tmp == 0.0 ? 1.0 : tmp);
    }

    void WriteOstream(std::ostream& os) const override {
        std::ios::fmtflags flags(os.flags());

        os << "counter " << std::fixed << std::setw(10) << std::setprecision(10)
            << std::setfill('0') << GetRatioCounter() << ";" <<
            "total " << std::fixed << std::setw(10) << std::setprecision(10)
            << std::setfill('0') << GetRatioTotalValue() << ";" <<
            "min " << std::fixed << std::setw(10) << std::setprecision(10)
            << std::setfill('0') << GetRatioMinValue() << ";" <<
            "max " << std::fixed << std::setw(10) << std::setprecision(10)
            << std::setfill('0') << GetRatioMaxValue();

        os.flags(flags);
    }

private:
    const bool m_refs;
    Unit* m_unit1;
    Unit* m_unit2;
};

}
}

#endif //IBNET_STATS_RATIO_HPP
