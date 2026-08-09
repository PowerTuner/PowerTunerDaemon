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
#include "pwtShared/Include/CPU/AMD/PStateCurrentLimit.h"
#include "pwtShared/Include/Types/ROData.h"

namespace PWTD::AMD {
    class MSR_PSTATE_CURRENT_LIMIT final: public CPURegister {
    private:
        static constexpr uint32_t addr = 0xc0010061;

    public:
        [[nodiscard]]
        PWTS::ROData<PWTS::AMD::PStateCurrentLimit> getPStateCurrentLimitData() const {
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, 0))
                return {};

            return PWTS::ROData<PWTS::AMD::PStateCurrentLimit>({
                .curPStateLimit = static_cast<int>(getBitfield(3, 0, reg)),
                .pstateMaxValue = static_cast<int>(getBitfield(7, 4, reg))
            }, true);
        }
    };
}
