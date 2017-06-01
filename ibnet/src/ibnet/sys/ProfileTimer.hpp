#ifndef IBNET_SYS_PROFILETIMER_HPP
#define IBNET_SYS_PROFILETIMER_HPP

#include <chrono>
#include <cstdint>
#include <iostream>

namespace ibnet {
namespace sys {

class ProfileTimer
{
public:
    ProfileTimer(const std::string& name = "") :
        m_name(name),
        m_counter(0),
        m_total(std::chrono::nanoseconds(0)),
        m_worst(std::chrono::seconds(0)),
        // over 10 years is enough...
        m_best(std::chrono::hours(10000))
    {};

    ~ProfileTimer(void) {};

    void Reset(void)
    {
        m_counter = 0;
        m_total = std::chrono::nanoseconds(0);
        m_worst = std::chrono::seconds(0);
        // over 10 years is enough...
        m_best = std::chrono::hours(10000);
    }

    void Enter(void)
    {
        m_counter++;
        m_enter = std::chrono::high_resolution_clock::now();
    }

    void Exit(void)
    {
        std::chrono::duration<uint64_t, std::nano> delta(std::chrono::high_resolution_clock::now() - m_enter);
        m_total += delta;

        if (delta < m_best) {
            m_best = delta;
        }

        if (delta > m_worst) {
            m_worst = delta;
        }
    }

    uint64_t GetCounter(void) const {
        return m_counter;
    }

    double GetTotalTime(void) const {
        return std::chrono::duration<double>(m_total).count();
    }

    template<typename _Unit>
    double GetTotalTime(void) const {
        return std::chrono::duration<double, _Unit>(m_total).count();
    }

    double GetAvarageTime(void) const
    {
        if (m_counter == 0) {
            return 0;
        } else {
            return std::chrono::duration<double>(m_total).count() / m_counter;
        }
    }

    template<typename _Unit>
    double GetAvarageTime(void) const
    {
        if (m_counter == 0) {
            return 0;
        } else {
            return std::chrono::duration<double, _Unit>(m_total).count() / m_counter;
        }
    }

    double GetBestTime(void) const {
        return std::chrono::duration<double>(m_best).count();
    }

    template<typename _Unit>
    double GetBestTime(void) const {
        return std::chrono::duration<double, _Unit>(m_best).count();
    }

    double GetWorstTime(void) const {
        return std::chrono::duration<double>(m_worst).count();
    }

    template<typename _Unit>
    double GetWorstTime(void) const {
        return std::chrono::duration<double, _Unit>(m_worst).count();
    }

    friend std::ostream &operator<<(std::ostream& os, const ProfileTimer& o) {
        return os <<
            o.m_name <<
            " counter: " << std::dec << o.m_counter <<
            ", total: " << o.GetTotalTime() <<
            " sec, avg: " << o.GetAvarageTime() <<
            " sec, best: " << o.GetBestTime() <<
            " sec, worst: " << o.GetWorstTime() <<
            " sec";
    }

private:
    const std::string m_name;
    uint64_t m_counter;
    std::chrono::high_resolution_clock::time_point m_enter;

    std::chrono::duration<uint64_t, std::nano> m_total;
    std::chrono::duration<uint64_t, std::nano> m_best;
    std::chrono::duration<uint64_t, std::nano> m_worst;
};

}
}

#endif //IBNET_SYS_PROFILETIMER_HPP
