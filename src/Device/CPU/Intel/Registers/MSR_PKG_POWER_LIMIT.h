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
#include "MSR_RAPL_POWER_UNIT.h"
#include "pwtShared/Include/CPU/Intel/PkgPowerLimit.h"
#include "pwtShared/Include/Types/RWData.h"
#include "../IntelUtils.h"

namespace PWTD::Intel {
    class MSR_PKG_POWER_LIMIT final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x610;

    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::PkgPowerLimit> get(const std::optional<MSR_RAPL_POWER_UNIT::Units> &pu) const {
            uint64_t reg;

            if (!pu || !msrDev->read(addr, 0, reg))
                return {};

            return PWTS::RWData<PWTS::Intel::PkgPowerLimit>({
                .pl1 = static_cast<int>(static_cast<double>(getBitfield(14, 0, reg)) * pu->power * 1000),
                .pl2 = static_cast<int>(static_cast<double>(getBitfield(46, 32, reg)) * pu->power * 1000),
                .pl1Time = static_cast<int>(std::pow(2, getBitfield(21, 17, reg)) * (1 + static_cast<double>(getBitfield(23, 22, reg)) / 4.0) * pu->time * 1000),
                .pl2Time = static_cast<int>(std::pow(2, getBitfield(53, 49, reg)) * (1 + static_cast<double>(getBitfield(55, 54, reg)) / 4.0) * pu->time * 1000),
                .pl1Clamp = getBitfield(16, 16, reg) == 1,
                .pl2Clamp = getBitfield(48, 48, reg) == 1,
                .pl1Enable = getBitfield(15, 15, reg) == 1,
                .pl2Enable = getBitfield(47, 47, reg) == 1,
                .lock = getBitfield(63, 63, reg) == 1
            }, true);
        }

        [[nodiscard]]
        bool set(const PWTS::RWData<PWTS::Intel::PkgPowerLimit> &data, const std::optional<MSR_RAPL_POWER_UNIT::Units> &pu) const {
            uint64_t reg;

            if (!data.isValid())
                return true;

            if (!pu || !msrDev->read(addr, 0, reg))
                return false;

            const PWTS::Intel::PkgPowerLimit pkgPowerLim = data.getValue();
            const std::optional<std::pair<int, int>> pl1Window = getRawTimeWindow(static_cast<double>(pkgPowerLim.pl1Time) / 1000, pu->time);
            const std::optional<std::pair<int, int>> pl2Window = getRawTimeWindow(static_cast<double>(pkgPowerLim.pl2Time) / 1000, pu->time);
            const int pl1 = static_cast<int>(pkgPowerLim.pl1 / pu->power / 1000);
            const int pl2 = static_cast<int>(pkgPowerLim.pl2 / pu->power / 1000);

            reg = setBitfield(14, 0, pl1, reg);
            reg = setBitfield(15, 15, pkgPowerLim.pl1Enable, reg);
            reg = setBitfield(16, 16, pkgPowerLim.pl1Clamp, reg);
            reg = setBitfield(46, 32, pl2, reg);
            reg = setBitfield(47, 47, pkgPowerLim.pl2Enable, reg);
            reg = setBitfield(48, 48, pkgPowerLim.pl2Clamp, reg);
            reg = setBitfield(63, 63, pkgPowerLim.lock, reg);

            if (pl1Window) {
                const auto [y, z] = pl1Window.value();

                reg = setBitfield(21, 17, y, reg);
                reg = setBitfield(23, 22, z, reg);
            }

            if (pl2Window) {
                const auto [y, z] = pl2Window.value();

                reg = setBitfield(53, 49, y, reg);
                reg = setBitfield(55, 54, z, reg);
            }

            if (!msrDev->write(reg, addr, 0) || !msrDev->read(addr, 0, reg))
                return false;

            return getBitfield(14, 0, reg) == pl1 &&
                getBitfield(15, 15, reg) == pkgPowerLim.pl1Enable &&
                getBitfield(16, 16, reg) == pkgPowerLim.pl1Clamp &&
                getBitfield(46, 32, reg) == pl2 &&
                getBitfield(47, 47, reg) == pkgPowerLim.pl2Enable &&
                getBitfield(48, 48, reg) == pkgPowerLim.pl2Clamp &&
                getBitfield(63, 63, reg) == pkgPowerLim.lock;
        }
    };
}
