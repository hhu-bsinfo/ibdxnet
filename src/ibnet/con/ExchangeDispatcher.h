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

#ifndef IBNET_CON_EXCHANGEDISPATCHER_H
#define IBNET_CON_EXCHANGEDISPATCHER_H

#include "ExchangeManager.h"

namespace ibnet {
namespace con {

// forward declaration for friendship
class ExchangeManager;

//
// Created by nothaas on 1/29/18.
//
class ExchangeDispatcher
{
public:
    friend class ExchangeManager;

protected:
    ExchangeDispatcher() = default;

    virtual ~ExchangeDispatcher() = default;

    virtual void _DispatchExchangeData(uint32_t sourceIPV4,
        const ExchangeManager::PaketHeader* paketHeader,
        const void* data) = 0;
};

}
}

#endif //IBNET_CON_JOBDISPATCHER_H
