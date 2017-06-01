#ifndef IBNET_MSG_BUFFERPOOL_H
#define IBNET_MSG_BUFFERPOOL_H

#include "ibnet/core/IbProtDom.h"

namespace ibnet {
namespace msg {

class BufferPool
{
public:
    struct Entry
    {
        uint32_t m_id;
        std::shared_ptr<core::IbMemReg> m_mem;

        Entry(uint32_t id, std::shared_ptr<core::IbMemReg> mem) :
            m_id(id),
            m_mem(mem)
        {}
    };

    BufferPool(uint32_t bufferSize, uint32_t poolSize, std::shared_ptr<core::IbProtDom>& protDom);
    ~BufferPool(void);

    const std::vector<Entry>& GetEntries(void) const {
        return m_pool;
    }

    const Entry& Get(uint32_t index) {
        return m_pool.at(index);
    }

private:
    const uint32_t m_bufferSize;

    std::shared_ptr<core::IbProtDom> m_protDom;
    std::vector<Entry> m_pool;
};

}
}

#endif //IBNET_MSG_BUFFERPOOL_H
