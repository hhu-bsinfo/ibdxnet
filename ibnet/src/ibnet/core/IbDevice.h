#ifndef IBNET_CORE_IBDEVICE_H
#define IBNET_CORE_IBDEVICE_H

#include <iostream>
#include <string>

#include <infiniband/verbs.h>
#include <spdlog/fmt/ostr.h>

namespace ibnet {
namespace core {

class IbDevice
{
public:
    enum PortState
    {
        e_PortStateInvalid = -1,
        e_PortStateNop = 0,
        e_PortStateDown = 1,
        e_PortStateInit = 2,
        e_PortStateArmed = 3,
        e_PortStateActive = 4,
        e_PortStateActiveDefer = 5,
    };

    enum MtuSize
    {
        e_MtuSizeInvalid = 0,
        e_MtuSize256 = 1,
        e_MtuSize512 = 2,
        e_MtuSize1024 = 3,
        e_MtuSize2048 = 4,
        e_MtuSize4096 = 5,
    };

    enum LinkWidth
    {
        e_LinkWidthInvalid = 0,
        e_LinkWidth1X = 1,
        e_LinkWidth4X = 4,
        e_LinkWidth8X = 8,
        e_LinkWidth12X = 12
    };

    enum LinkSpeed
    {
        e_LinkSpeedInvalid = 0,
        e_LinkSpeed2p5 = 25,
        e_LinkSpeed5 = 50,
        e_LinkSpeed10 = 100,
        e_LinkSpeed14 = 140,
        e_LinkSpeed25 = 250
    };

    enum LinkState
    {
        e_LinkStateInvalid = 0,
        e_LinkStateSleep = 1,
        e_LinkStatePolling = 2,
        e_LinkStateDisabled = 3,
        e_LinkStatePortConfTrain = 4,
        e_LinkStateLinkUp = 5,
        e_LinkStateLinkErrRecovery = 6,
        e_LinkStatePhytest = 7
    };

    IbDevice(void);
    ~IbDevice(void);

    void UpdateState(void);

    uint64_t GetGuid(void) const {
        return m_ibDevGuid;
    }

    const std::string& GetName(void) const {
        return m_ibDevName;
    }

    uint16_t GetLid(void) const {
        return m_lid;
    }

    LinkWidth GetLinkWidth(void) const {
        return m_linkWidth;
    }

    LinkSpeed GetLinkSpeed(void) const {
        return m_linkSpeed;
    }

    LinkState GetLinkState(void) const {
        return m_linkState;
    }

    ibv_context* GetIBContext(void) const {
        return m_ibCtx;
    }

    friend std::ostream &operator<<(std::ostream& os, const IbDevice& o) {
        return os << "0x" << std::hex << o.m_ibDevGuid
                  << ", " << o.m_ibDevName
                  << ", " << std::hex << "0x" << o.m_lid
                  << ", " << o.m_linkWidth << "X"
                  << ", " << o.m_linkSpeed / 10.f << " gbps"
                  << ", MaxMTU " << ms_mtuSizeStr[o.m_maxMtuSize]
                  << ", ActiveMTU " << ms_mtuSizeStr[o.m_maxMtuSize]
                  << ", Port " << ms_portStateStr[o.m_portState]
                  << ", Link " << ms_linkStateStr[o.m_linkState];
    }

private:
    static const std::string ms_portStateStr[7];
    static const std::string ms_mtuSizeStr[6];
    static const std::string ms_linkStateStr[8];

private:
    uint64_t m_ibDevGuid;
    std::string m_ibDevName;
    uint16_t m_lid;

    PortState m_portState;
    MtuSize m_maxMtuSize;
    MtuSize m_activeMtuSize;
    LinkWidth m_linkWidth;
    LinkSpeed m_linkSpeed;
    LinkState m_linkState;

    ibv_context* m_ibCtx;

    void __LogDeviceAttributes(void);
};

}
}

#endif // IBNET_CORE_IBDEVICE_H
