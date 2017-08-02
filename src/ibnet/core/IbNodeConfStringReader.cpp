#include "IbNodeConfStringReader.h"

#include "ibnet/sys/StringUtils.h"

namespace ibnet {
namespace core {

IbNodeConfStringReader::IbNodeConfStringReader(const std::string& str) :
    m_str(str)
{

}

IbNodeConfStringReader::~IbNodeConfStringReader(void)
{

}

IbNodeConf IbNodeConfStringReader::Read(void)
{
    IbNodeConf conf;
    std::vector<std::string> tokens = sys::StringUtils::Split(m_str);

    for (auto& it : tokens) {
        conf.AddEntry(it);
    }

    return conf;
}

}
}