#include "IbNodeConfArgListReader.h"

namespace ibnet {
namespace core {

IbNodeConfArgListReader::IbNodeConfArgListReader(uint32_t numItems,
        char** args) :
    IbNodeConfReader(),
    m_numItems(numItems),
    m_args(args)
{

}

IbNodeConfArgListReader::~IbNodeConfArgListReader(void)
{

}

IbNodeConf IbNodeConfArgListReader::Read(void)
{
    IbNodeConf conf;

    for (uint32_t i = 0; i < m_numItems; i++) {
        conf.AddEntry(m_args[i]);
    }

    return conf;
}

}
}