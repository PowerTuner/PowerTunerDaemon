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

namespace PWTD::Intel {
    class MSR_PP1_POLICY final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x642;

    public:
        [[nodiscard]]
        std::optional<int> get() const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return static_cast<int>(getBitfield(4, 0, reg));
        }

        [[nodiscard]]
        bool set(const int priority) const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(4, 0, priority, reg);

            return msrDev->write(reg, addr, 0) &&
                msrDev->read(addr, 0, reg) &&
                getBitfield(4, 0, reg) == priority;
        }
    };
}
