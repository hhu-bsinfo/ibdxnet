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

#include "Random.h"

namespace ibnet {
namespace sys {

std::random_device* Random::ms_rd = nullptr;
std::mt19937* Random::ms_mt = nullptr;

void Random::Init()
{
    ms_rd = new std::random_device();
    ms_mt = new std::mt19937(ms_rd->operator()());
}

void Random::Shutdown()
{
    delete ms_mt;
    ms_mt = nullptr;
    delete ms_rd;
    ms_rd = nullptr;
}

uint16_t Random::Generate16()
{
    static std::uniform_int_distribution<uint16_t> dist;
    return dist(*ms_mt);
}

uint32_t Random::Generate32()
{
    static std::uniform_int_distribution<uint32_t> dist;
    return dist(*ms_mt);
}

}
}