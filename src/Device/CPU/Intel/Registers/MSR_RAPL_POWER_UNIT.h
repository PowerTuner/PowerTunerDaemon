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
#include "pwtShared/Include/Types/ROData.h"

namespace PWTD::Intel {
    class MSR_RAPL_POWER_UNIT final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x606;

    public:
        struct [[nodiscard]] RAPLPowerUnits final {
            double powerUnit;
            double timeUnit;
        };

        [[nodiscard]]
        PWTS::ROData<RAPLPowerUnits> getPowerUnitData() const {
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, 0))
                return {};

            return PWTS::ROData<RAPLPowerUnits>({
                .powerUnit = 1 / std::pow(2, getBitfield(3, 0, reg)),
                .timeUnit = 1 / std::pow(2, getBitfield(19, 16, reg))
            }, true);
        }
    };
}