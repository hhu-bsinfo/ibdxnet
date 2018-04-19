/*
 * Copyright (C) 2018 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef IBNET_MSGRC_SCATTERGATHERLIST_H
#define IBNET_MSGRC_SCATTERGATHERLIST_H

#include <cstdint>

#include <infiniband/verbs.h>

#include "ibnet/sys/IllegalStateException.h"

#include "ibnet/core/IbMemReg.h"

namespace ibnet {
namespace msgrc {

/**
 * Wrapper with helper methods for managing scatter gather elements
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 22.03.2018
 */
struct ScatterGatherList
{
    const uint32_t m_maxSges;

    uint32_t m_numUsedElems;
    core::IbMemReg** m_refsMemReg;
    ibv_sge* m_sgeList;

    /**
     * Constructor
     *
     * @param maxSge Max number of SGEs for a single list
     */
    ScatterGatherList(uint32_t maxSge) :
        m_maxSges(maxSge),
        m_numUsedElems(0),
        m_refsMemReg(new core::IbMemReg*[maxSge]),
        m_sgeList(new ibv_sge[maxSge])
    {
    };

    /**
     * Destructor
     */
    ~ScatterGatherList()
    {
        delete [] m_refsMemReg;
        delete [] m_sgeList;
    }

    /**
     * Add a memory region to the SGE list
     *
     * @param refMemReg Ref to a memory region to add to the list (memory managed by caller)
     */
    void Add(core::IbMemReg* refMemReg)
    {
        if (m_numUsedElems >= m_maxSges) {
            throw sys::IllegalStateException("List overflow: %d", m_maxSges);
        }

        m_refsMemReg[m_numUsedElems] = refMemReg;

        m_sgeList[m_numUsedElems].addr = (uintptr_t) refMemReg->GetAddress();
        m_sgeList[m_numUsedElems].length = refMemReg->GetSizeBuffer();
        m_sgeList[m_numUsedElems].lkey = refMemReg->GetLKey();

        m_numUsedElems++;
    }

    /**
     * Clear the list
     */
    void Reset()
    {
        m_numUsedElems = 0;
    }

    /**
     * Enable output to an out stream
     */
    friend std::ostream& operator<<(std::ostream& os, const ScatterGatherList& o)
    {
        os << "m_numUsedElems " << o.m_numUsedElems;

        for (uint32_t i = 0; i < o.m_numUsedElems; i++) {
            os << ", (" << i << ") " << *o.m_refsMemReg[i];
        }

        return os;
    }
};

}
}

#endif //IBNET_MSGRC_SCATTERGATHERLIST_H
