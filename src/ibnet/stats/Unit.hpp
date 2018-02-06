//
// Created by nothaas on 2/1/18.
//

#ifndef IBNET_STATS_UNIT_HPP
#define IBNET_STATS_UNIT_HPP

#include <cmath>
#include <cstdint>

#include "Operation.hpp"

namespace ibnet {
namespace stats {

class Unit : public Operation
{
public:
    enum Base
    {
        e_Base10 = 1000,
        e_Base2 = 1024
    };

    enum Metric
    {
        e_MetricDefault = 0,
        e_MetricKilo = 1,
        e_MetricMega = 2,
        e_MetricGiga = 3,
        e_MetricTera = 4,
        e_MetricPeta = 5,
        e_MetricExa = 6,
        e_MetricMax = 7,
        e_MetricCount = 8
    };

public:
    explicit Unit(const std::string& name, Base base = e_Base10) :
        Operation(name),
        m_base(base),
        m_metricTable(),
        m_total(0),
        m_counter(0),
        m_min(0xFFFFFFFFFFFFFFFF),
        m_max(0)
    {
        m_metricTable[0] = 1;

        for (uint8_t i = e_MetricKilo; i < e_MetricMax; i++) {
            m_metricTable[i] = 1;
            for (uint8_t j = 0; j < i; j++) {
                m_metricTable[i] *= static_cast<uint64_t>(base);
            }
        }

        m_metricTable[e_MetricMax] = m_metricTable[e_MetricMax - 1];
    }

    ~Unit() override = default;

    inline uint64_t GetMetricFactor(Metric metric) {
        return m_metricTable[metric];
    }

    inline uint64_t GetCounter() const {
        return m_counter;
    }

    inline double GetCounter(Metric metric) const {
        return GetCounter() / m_metricTable[metric];
    }

    inline uint64_t GetTotalValue() const {
        return m_total;
    }

    inline double GetTotalValue(Metric metric) const {
        return GetTotalValue() / m_metricTable[metric];
    }

    inline double GetAvgValue() const {
        return m_counter == 0 ? 0 : (double) m_total / m_counter;
    }

    inline double GetAvgValue(Metric metric) const {
        return GetAvgValue() / m_metricTable[metric];
    }

    inline uint64_t GetMinValue() const {
        return m_min;
    }

    inline double GetMinValue(Metric metric) const {
        return GetMinValue() / m_metricTable[metric];
    }

    inline uint64_t GetMaxValue() const {
        return m_max;
    }

    inline uint64_t GetMaxValue(Metric metric) const {
        return GetMaxValue() / m_metricTable[metric];
    }

    inline void Inc() {
        Add(1);
    }

    inline void Add(uint64_t val) {
        m_counter++;
        m_total += val;

        if (m_min > val) {
            m_min = val;
        }

        if (m_max < val) {
            m_max = val;
        }
    }

    void WriteOstream(std::ostream& os) const override {
        os << "counter " << GetCounter();
        __FormatUnit(os, "total", GetTotalValue());
        __FormatUnit(os, "avg", GetAvgValue());
        __FormatUnit(os, "min", GetMinValue());
        __FormatUnit(os, "max", GetMaxValue());
    }

private:
    const Base m_base;
    uint64_t m_metricTable[e_MetricCount];
    static const char* ms_metricTableNames[];

private:
    uint64_t m_counter;
    uint64_t m_total;

    uint64_t m_min;
    uint64_t m_max;

private:
    inline void __FormatUnit(std::ostream& os, const std::string& name,
            uint64_t units) const {
        Metric metric;

        for (uint8_t i = 1; i < e_MetricCount; i++) {
            if (units < m_metricTable[i]) {
                os << ";" << name << " " <<
                    (double) units / m_metricTable[i - 1] << " " <<
                     ms_metricTableNames[i - 1];
                break;
            }
        }
    }
};

}
}

#endif //IBNET_STATS_UNIT_HPP
