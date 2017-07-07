#include "BufferPool.h"

namespace ibnet {
namespace dx {

BufferPool::BufferPool(uint32_t bufferSize, uint32_t poolSize, std::shared_ptr<core::IbProtDom>& protDom) :
    m_bufferSize(bufferSize),
    m_protDom(protDom),
    m_pool()
{
    for (uint32_t i = 0; i < poolSize; i++) {
        void* mem = malloc(bufferSize);
        m_pool.push_back(Entry(i, m_protDom->Register(mem, bufferSize, true)));
    }
}

BufferPool::~BufferPool(void)
{

}

}
}