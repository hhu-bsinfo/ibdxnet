/*
 * Copyright (C) 2018 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef IBNET_STATS_UNIT_HPP
#define IBNET_STATS_UNIT_HPP

#include <cmath>
#include <cstdint>
#include <iomanip>

#include "Operation.hpp"

namespace ibnet {
namespace stats {

/**
 * Statistic operation tracking a value/unit
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.02.2018
 */
class Unit : public Operation
{
public:
    /**
     * Base of the unit
     */
    enum Base
    {
        e_Base10 = 1000,
        e_Base2 = 1024
    };

    /**
     * Metric of the unit
     */
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
    /**
     * Constructor
     *
     * @param category Name for the category (for sorting), e.g. class name
     * @param name Name of the statistic operation
     * @param base Base to use for converting to different metrics
     */
    explicit Unit(const std::string& category, const std::string& name, Base base = e_Base10) :
            Operation(category, name),
            m_base(base),
            m_metricTable(),
            m_counter(0),
            m_total(0),
            m_min(0xFFFFFFFFFFFFFFFF),
            m_max(0)
    {
        for (uint8_t i = e_MetricDefault; i < e_MetricMax; i++) {
            m_metricTable[i] = 1;

            for (uint8_t j = 0; j < i; j++) {
                m_metricTable[i] *= static_cast<uint64_t>(base);
            }
        }

        m_metricTable[e_MetricMax] = m_metricTable[e_MetricMax - 1];
    }

    ~Unit() override = default;

    /**
     * Get the factor applied when converting
     *
     * @param metric Metric to convert to
     * @return Factor applied to value
     */
    inline uint64_t GetMetricFactor(Metric metric)
    {
        return m_metricTable[metric];
    }

    /**
     * Get the number of times this value was modified
     */
    inline uint64_t GetCounter() const
    {
        return m_counter;
    }

    /**
     * Get the number of times this value was modified
     *
     * @param metric Metric to convert to
     */
    inline double GetCounter(Metric metric) const
    {
        return GetCounter() / static_cast<double>(m_metricTable[metric]);
    }

    /**
     * Get the (total) value
     */
    inline uint64_t GetTotalValue() const
    {
        return m_total;
    }

    /**
     * Get the (total) value
     *
     * @param metric Metric to convert to
     */
    inline double GetTotalValue(Metric metric) const
    {
        return GetTotalValue() / static_cast<double>(m_metricTable[metric]);
    }

    /**
     * Get the average value: total / counter
     */
    inline double GetAvgValue() const
    {
        return m_counter == 0 ? 0.0 : static_cast<double>(m_total) / m_counter;
    }

    /**
     * Get the average value: total / counter
     *
     * @param metric Metric to convert to
     */
    inline double GetAvgValue(Metric metric) const
    {
        return GetAvgValue() / static_cast<double>(m_metricTable[metric]);
    }

    /**
     * Get the minimum value ever applied/added to the total value
     */
    inline uint64_t GetMinValue() const
    {
        return m_min;
    }

    /**
     * Get the minimum value ever applied/added to the total value
     *
     * @param metric Metric to convert to
     */
    inline double GetMinValue(Metric metric) const
    {
        return GetMinValue() / static_cast<double>(m_metricTable[metric]);
    }

    /**
     * Get the maximum value ever applied/added to the total value
     */
    inline uint64_t GetMaxValue() const
    {
        return m_max;
    }

    /**
     * Get the maximum value ever applied/added to the total value
     *
     * @param metric Metric to convert to
     */
    inline uint64_t GetMaxValue(Metric metric) const
    {
        return GetMaxValue() / m_metricTable[metric];
    }

    /**
     * Incrememnt the value by 1
     */
    inline void Inc()
    {
        Add(1);
    }

    /**
     * Add a value to the total
     *
     * @param val Value to add to the current total
     */
    inline void Add(uint64_t val)
    {
        m_counter++;
        m_total += val;

        if (m_min > val) {
            m_min = val;
        }

        if (m_max < val) {
            m_max = val;
        }
    }

    /**
     * Overriding virtual method
     */
    void WriteOstream(std::ostream& os, const std::string& indent) const override
    {
        os << indent << "counter " << GetCounter();
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
            double units) const
    {
        for (uint8_t i = 1; i < e_MetricCount; i++) {
            if (units < m_metricTable[i]) {
                std::ios::fmtflags f(os.flags());

                os << ";" << name << " " << std::setprecision(3) << std::fixed <<
                        (double) units / m_metricTable[i - 1] << " " << ms_metricTableNames[i - 1];

                os.flags(f);

                break;
            }
        }
    }
};

}
}

#endif //IBNET_STATS_UNIT_HPP
