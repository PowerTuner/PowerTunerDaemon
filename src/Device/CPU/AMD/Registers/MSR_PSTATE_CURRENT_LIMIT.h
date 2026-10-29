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
#include "../../../../external/libPowerTuner/src/Vendor/AMD/Include/PStateCurrentLimit.h"

namespace PWTD::AMD {
    class MSR_PSTATE_CURRENT_LIMIT final: public CPURegister {
    private:
        static constexpr uint32_t addr = 0xc0010061;

    public:
        [[nodiscard]]
        std::optional<LIBPWT::AMD::PStateCurrentLimit> get() const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return LIBPWT::AMD::PStateCurrentLimit {
                .curPStateLimit = static_cast<int>(getBitfield(3, 0, reg)),
                .pstateMaxValue = static_cast<int>(getBitfield(7, 4, reg))
            };
        }
    };
}
