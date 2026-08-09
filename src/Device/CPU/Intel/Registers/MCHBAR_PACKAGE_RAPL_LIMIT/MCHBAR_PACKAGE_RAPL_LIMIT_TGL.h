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
#include "MCHBAR_PACKAGE_RAPL_LIMIT.h"

namespace PWTD::Intel {
    class MCHBAR_PACKAGE_RAPL_LIMIT_TGL final: public MCHBAR_PACKAGE_RAPL_LIMIT {
    public:
        explicit MCHBAR_PACKAGE_RAPL_LIMIT_TGL(const uint32_t base): MCHBAR_PACKAGE_RAPL_LIMIT(base) {}

        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::MCHBARPkgRaplLimit> get(const std::optional<MCHBAR_PACKAGE_POWER_SKU_UNIT::Units> &pu) const override {
            uint64_t reg;

            if (!pu || !memory->readMem64(reg, addr))
                return {};

            return PWTS::RWData<PWTS::Intel::MCHBARPkgRaplLimit>({
                .pl1 = static_cast<int>(static_cast<double>(getBitfield(14, 0, reg)) * pu->power * 1000),
                .pl2 = static_cast<int>(static_cast<double>(getBitfield(46, 32, reg)) * pu->power * 1000),
                .pl1Time = static_cast<int>(std::pow(2, getBitfield(21, 17, reg)) * (1 + static_cast<double>(getBitfield(23, 22, reg)) / 4.0) * pu->time * 1000),
                .pl1Clamp = getBitfield(16, 16, reg) == 1,
                .pl1Enable = getBitfield(15, 15, reg) == 1,
                .pl2Enable = getBitfield(47, 47, reg) == 1,
                .lock = getBitfield(63, 63, reg) == 1
            }, true);
        }

        [[nodiscard]]
        bool set(const PWTS::RWData<PWTS::Intel::MCHBARPkgRaplLimit> &data, const std::optional<MCHBAR_PACKAGE_POWER_SKU_UNIT::Units> &pu) const override {
            uint64_t reg;

            if (!data.isValid())
                return true;

            if (!pu || !memory->readMem64(reg, addr))
                return false;

            const PWTS::Intel::MCHBARPkgRaplLimit pkgPowerLim = data.getValue();
            const int pl1 = static_cast<int>(pkgPowerLim.pl1 / pu->power / 1000);
            const int pl2 = static_cast<int>(pkgPowerLim.pl2 / pu->power / 1000);
            const std::optional<std::pair<int, int>> pl1Window = getRawTimeWindow(static_cast<double>(pkgPowerLim.pl1Time) / 1000, pu->time);

            reg = setBitfield(14, 0, pl1, reg);
            reg = setBitfield(15, 15, pkgPowerLim.pl1Enable, reg);
            reg = setBitfield(16, 16, pkgPowerLim.pl1Clamp, reg);
            reg = setBitfield(46, 32, pl2, reg);
            reg = setBitfield(47, 47, pkgPowerLim.pl2Enable, reg);
            reg = setBitfield(63, 63, pkgPowerLim.lock, reg);

            if (pl1Window) {
                const auto [y, z] = pl1Window.value();

                reg = setBitfield(21, 17, y, reg);
                reg = setBitfield(23, 22, z, reg);
            }

            if (!memory->writeMem64(reg, addr) || !memory->readMem64(reg, addr))
                return false;

            return getBitfield(14, 0, reg) == pl1 &&
                getBitfield(15, 15, reg) == pkgPowerLim.pl1Enable &&
                getBitfield(16, 16, reg) == pkgPowerLim.pl1Clamp &&
                getBitfield(46, 32, reg) == pl2 &&
                getBitfield(47, 47, reg) == pkgPowerLim.pl2Enable &&
                getBitfield(63, 63, reg) == pkgPowerLim.lock;
        }
    };
}
