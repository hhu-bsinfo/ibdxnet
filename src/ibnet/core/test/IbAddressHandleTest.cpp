/*
 * Copyright (C) 2017 Heinrich-Heine-Universitaet Duesseldorf,
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

#include <iostream>

#include "ibnet/sys/Logger.h"
#include "ibnet/core/IbGlobalRoutingHeader.h"
#include "ibnet/core/IbProtDom.h"
#include "ibnet/core/IbDevice.h"
#include "ibnet/core/IbAddressHandle.h"

int main(int argc, char** argv)
{
    ibnet::sys::Logger::Setup();
    
    ibnet::core::IbDevice ibDevice;
    ibnet::core::IbProtDom ibProtDom(ibDevice, "ahTestDomain");
    ibnet::core::IbGlobalRoutingHeader ibGrh(0, 0, 0, 0, 0, 0);
    
    ibnet::core::IbAddressHandle ibAh1(ibGrh, ibProtDom, 0, 0, 0, 0, 1);
    std::shared_ptr<ibnet::core::IbAddressHandle> ibAh2 = std::make_shared<ibnet::core::IbAddressHandle>(ibProtDom, 0, 0, 0, 0, 1);

    return 0;
}
