/*
 * This file is part of PowerTunerDaemon.
 * Copyright (C) 2025 kylon
 *
 * PowerTunerDaemon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PowerTunerDaemon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include "../../CPURegister.h"
#include "../../../Utils/Utils.h"
#include "pwtShared/Include/CPU/Intel/HWPCapabilities.h"
#include "pwtShared/Include/Types/ROData.h"

namespace PWTD::Intel {
    class IA32_HWP_CAPABILITIES final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x771;

    public:
        [[nodiscard]]
        PWTS::ROData<PWTS::Intel::HWPCapabilities> get(const int cpu) const {
            uint64_t reg;

            if (!msrDev->read(addr, cpu, reg))
                return {};

            return PWTS::ROData<PWTS::Intel::HWPCapabilities>({
                .lowestPerf = static_cast<int>(getBitfield(31, 24, reg)),
                .highestPerf = static_cast<int>(getBitfield(7, 0, reg))
            }, true);
        }
    };
}
