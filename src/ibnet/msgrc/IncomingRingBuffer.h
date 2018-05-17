//
// Created by nothaas on 5/17/18.
//

#ifndef IBNET_MSGRC_INCOMINGDATAPOOL_H
#define IBNET_MSGRC_INCOMINGDATAPOOL_H

#include "ibnet/core/IbMemReg.h"

#include "ibnet/con/NodeId.h"

namespace ibnet {
namespace msgrc {

class IncomingRingBuffer
{
public:
    /**
     * Ring buffer with information about received data. We need this as a separate struct
     * in order to map it to Java using unsafe
     */
    struct RingBuffer
    {
        /**
         * Get the size of the struct
         *
         * @param maxCount Max number of entry elements
         * @return Size of the struct depending on the max num of entry elements
         */
        static size_t Sizeof(uint32_t maxCount)
        {
            return 4 * sizeof(uint32_t) + maxCount * sizeof(Entry);
        }

        uint32_t m_usedEntries;
        uint32_t m_front;
        uint32_t m_back;
        uint32_t m_size;

        /**
         * Single receive entry. If receiving data from multiple nodes,
         * multiple entries are used in the receive package.
         */
        struct Entry
        {
            con::NodeId m_sourceNodeId;
            uint8_t m_fcData;
            // ensure proper alignment
            uint8_t m_padding;
            uint32_t m_dataLength;
            core::IbMemReg* m_data;
            void* m_dataRaw;
        } __attribute__((__packed__)) m_entries[];
    } __attribute__((__packed__));

    IncomingRingBuffer(uint32_t size);
    ~IncomingRingBuffer();

    bool IsFull() const {
        return m_buffer->m_usedEntries == m_buffer->m_size;
    }

    bool IsEmpty() const {
        return m_buffer->m_usedEntries == 0;
    }

    uint32_t NumUsedEntries() const {
        return m_buffer->m_usedEntries;
    }

    uint32_t NumFreeEntries() const {
        return m_buffer->m_size - m_buffer->m_usedEntries;
    }

    RingBuffer::Entry* Front() {
        return &m_buffer->m_entries[m_buffer->m_front];
    }

    void PopFront(uint32_t count);

    RingBuffer::Entry* Back() {
        return &m_buffer->m_entries[m_buffer->m_back];
    }

    void PushBack();

    const RingBuffer* GetRingBuffer() const {
        return m_buffer;
    }

    std::string ToString() const;

    /**
     * Overloading << operator for printing to ostreams
     *
     * @param os Ostream to output to
     * @param o Operation to generate output for
     * @return Ostream object
     */
    friend std::ostream& operator<<(std::ostream& os, const IncomingRingBuffer& o)
    {
        return os << o.ToString();
    }

private:
    // contains the ring buffer = entry array
    RingBuffer* m_buffer;
};

}
}

#endif //IBNET_MSGRC_INCOMINGDATAPOOL_H
