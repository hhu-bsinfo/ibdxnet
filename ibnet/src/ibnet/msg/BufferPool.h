#ifndef IBNET_MSG_BUFFERPOOL_H
#define IBNET_MSG_BUFFERPOOL_H

#include "ibnet/core/IbProtDom.h"

namespace ibnet {
namespace msg {

/**
 * Pool for buffers that are registered with a protection domain
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class BufferPool
{
public:
    /**
     * Entry of the buffer pool
     */
    struct Entry
    {
        uint32_t m_id;
        std::shared_ptr<core::IbMemReg> m_mem;

        /**
         * Constructor
         *
         * @param id Unique id for the buffer (unique for the parent pool)
         * @param mem Memory region with buffer
         */
        Entry(uint32_t id, std::shared_ptr<core::IbMemReg> mem) :
            m_id(id),
            m_mem(mem)
        {}
    };

    /**
     * Constructor
     *
     * @param bufferSize Size per buffer in the pool in bytes
     * @param poolSize Number of buffers to pre-allocate for the pool
     * @param protDom Protection domain to register all buffers at
     */
    BufferPool(uint32_t bufferSize, uint32_t poolSize, std::shared_ptr<core::IbProtDom>& protDom);

    /**
     * Destructor
     */
    ~BufferPool(void);

    /**
     * Get all pool entries
     */
    const std::vector<Entry>& GetEntries(void) const {
        return m_pool;
    }

    /**
     * Get an entry by id
     *
     * @param index Index/id of the entry to get (max id = pool size - 1)
     * @return Buffer pool entry
     */
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
