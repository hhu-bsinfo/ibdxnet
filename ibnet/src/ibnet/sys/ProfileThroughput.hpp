#ifndef IBNET_SYS_PROFILETHROUGHPUT_HPP
#define IBNET_SYS_PROFILETHROUGHPUT_HPP

#include "ProfileTimer.hpp"

namespace ibnet {
namespace sys {

class ProfileThroughput
{
public:
    ProfileThroughput(uint32_t resSec = 1, const std::string& name = "") :
        m_resSec(resSec),
        m_name(name),
        m_totalTimer(),
        m_sliceTimer(),
        m_totalData(0),
        m_sliceData(0),
        m_updateCounter(0),
        m_lastThroughput(0),
        m_peakThroughput(0),
        m_avgCounter(0),
        m_avgThroughputSum(0)
    {

    }

    ~ProfileThroughput(void) {};

    void Start(void)
    {
        m_totalTimer.Enter();
        m_sliceTimer.Enter();
    }

    bool IsStarted(void) const {
        return m_totalTimer.GetCounter() > 0;
    }

    void Update(uint32_t bytes)
    {
        m_totalData += bytes;
        m_sliceData += bytes;
        m_updateCounter++;

        m_sliceTimer.Exit();
        if (m_sliceTimer.GetTotalTime() > m_resSec) {
            m_lastThroughput = m_sliceData / m_sliceTimer.GetTotalTime();

            if (m_peakThroughput < m_lastThroughput) {
                m_peakThroughput = m_lastThroughput;
            }

            m_avgCounter++;
            m_avgThroughputSum += m_lastThroughput;
        }
        m_sliceTimer.Enter();
    }

    uint64_t GetLastThroughput(void) const {
        return m_lastThroughput;
    }

    uint64_t GetCurrentPeak(void) const {
        return m_peakThroughput;
    }

    uint64_t GetCurrentAvg(void) const {
        if (m_avgCounter == 0) {
            return 0;
        }

        return m_avgThroughputSum / m_avgCounter;
    }

    uint64_t GetUpdateCount(void) const {
        return m_updateCounter;
    }

    void Stop(void)
    {
        m_sliceTimer.Exit();
        m_totalTimer.Exit();
    }

    friend std::ostream &operator<<(std::ostream& os,
            const ProfileThroughput& o) {
        return os << o.m_name <<
            " Total data: " << __FormatData(o.m_totalData) <<
            ", Last throughput: " << __FormatData(o.GetLastThroughput()) <<
            "/sec, Peak throughput: " << __FormatData(o.GetCurrentPeak()) <<
            "/sec, Avg throughput: " << __FormatData(o.GetCurrentAvg()) <<
            "/sec";
    }

private:
    uint32_t m_resSec;
    const std::string m_name;

    ProfileTimer m_totalTimer;
    ProfileTimer m_sliceTimer;

    uint64_t m_totalData;
    uint64_t m_sliceData;
    uint64_t m_updateCounter;

    uint64_t m_lastThroughput;
    uint64_t m_peakThroughput;
    uint64_t m_avgCounter;
    uint64_t m_avgThroughputSum;

    static std::string __FormatData(uint64_t bytes)
    {
        if (bytes < 1024) {
            return std::to_string(bytes) + " b";
        } else if (bytes < 1024 * 1024) {
            return std::to_string(bytes / 1024.0) + " kb";
        } else if (bytes < 1024 * 1024 * 1024) {
            return std::to_string(bytes / 1024.0 / 1024.0) + " mb";
        } else {
            return std::to_string(bytes / 1024.0 / 1024.0 / 1024.0) + " gb";
        }
    }
};

}
}

#endif //IBNET_SYS_PROFILETHROUGHPUT_HPP
