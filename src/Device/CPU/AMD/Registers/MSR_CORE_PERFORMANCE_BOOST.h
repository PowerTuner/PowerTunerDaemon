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

namespace PWTD::AMD {
    class MSR_CORE_PERFORMANCE_BOOST final: public CPURegister {
    private:
        static constexpr uint32_t addr = 0xc0010015;

    public:
        [[nodiscard]]
        std::optional<int> get(const int cpu) const {
            uint64_t reg;

            if (!msrDev->read(addr, cpu, reg))
                return {};

            return static_cast<int>(getBitfield(25, 25, reg));
        }

        [[nodiscard]]
        bool set(const int cpu, const int boost) const {
            uint64_t reg;

            if (!msrDev->read(addr, cpu, reg))
                return false;

            reg = setBitfield(25, 25, boost, reg);

            return msrDev->write(reg, addr, cpu) &&
                msrDev->read(addr, cpu, reg) &&
                getBitfield(25, 25, reg) == boost;
        }
    };
}
