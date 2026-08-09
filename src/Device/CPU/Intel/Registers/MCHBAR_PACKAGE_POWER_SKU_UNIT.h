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
#include "../MCHBAR/MCHBARRegister.h"
#include "../../../Utils/Utils.h"
#include "pwtShared/Include/Types/ROData.h"

namespace PWTD::Intel {
    class MCHBAR_PACKAGE_POWER_SKU_UNIT final: public MCHBARRegister {
    public:
        struct PkgPowerSKUUnits final {
            double powerUnit;
            double timeUnit;
        };

        explicit MCHBAR_PACKAGE_POWER_SKU_UNIT(const uint32_t base): MCHBARRegister(base, 0x5938) {}

        [[nodiscard]]
        PWTS::ROData<PkgPowerSKUUnits> getPkgSKUPowerUnitData() const {
            uint64_t reg;

            if (!memory->readMem32(reg, addr))
                return {};

            return PWTS::ROData<PkgPowerSKUUnits>({
                .powerUnit = 1 / std::pow(2, getBitfield(3, 0, reg)),
                .timeUnit = 1 / std::pow(2, getBitfield(19, 16, reg))
            }, true);
        }
    };
}
