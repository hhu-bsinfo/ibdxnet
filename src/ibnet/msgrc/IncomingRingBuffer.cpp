//
// Created by nothaas on 5/17/18.
//

#include "IncomingRingBuffer.h"

#include "ibnet/sys/IllegalStateException.h"

namespace ibnet {
namespace msgrc {

IncomingRingBuffer::IncomingRingBuffer(uint32_t size) :
    m_buffer(static_cast<RingBuffer*>(
            aligned_alloc(static_cast<size_t>(getpagesize()), RingBuffer::Sizeof(size))))
{
    // null everything
    memset(m_buffer, 0, RingBuffer::Sizeof(size));
    m_buffer->m_size = size;
}

IncomingRingBuffer::~IncomingRingBuffer()
{
    free(m_buffer);
}

void IncomingRingBuffer::PopFront(uint32_t count)
{
    if (count > m_buffer->m_usedEntries) {
        throw sys::IllegalStateException("IRB underflow, count %d: %s", count, ToString());
    }

    m_buffer->m_usedEntries -= count;
    m_buffer->m_front = (m_buffer->m_front + count) % m_buffer->m_size;
}

void IncomingRingBuffer::PushBack()
{
    m_buffer->m_usedEntries++;

    if (m_buffer->m_usedEntries > m_buffer->m_size) {
        throw sys::IllegalStateException("IRB overflow: %s", ToString());
    }

    m_buffer->m_back = (m_buffer->m_back + 1) % m_buffer->m_size;
}

std::string IncomingRingBuffer::ToString() const
{
    std::string str;

    str += "m_usedEntries " + std::to_string(m_buffer->m_usedEntries) + ", ";
    str += "m_front " + std::to_string(m_buffer->m_front) + ", ";
    str += "m_back " + std::to_string(m_buffer->m_back);

    return str;
}

}
}