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
#include "../../Utils/IntelRegisterUtils.h"

namespace PWTD::Intel {
    class MCHBAR_PACKAGE_RAPL_LIMIT_IVB final: public MCHBAR_PACKAGE_RAPL_LIMIT {
    public:
        explicit MCHBAR_PACKAGE_RAPL_LIMIT_IVB(const uint32_t base): MCHBAR_PACKAGE_RAPL_LIMIT(base) {}

        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::MCHBARPkgRaplLimit> getPkgRaplLimitData(const PWTS::ROData<MCHBAR_PACKAGE_POWER_SKU_UNIT::PkgPowerSKUUnits> &powerSkuUnit) const override {
            const MCHBAR_PACKAGE_POWER_SKU_UNIT::PkgPowerSKUUnits skuUnit = powerSkuUnit.getValue();
            uint64_t reg;

            if (!powerSkuUnit.isValid() || !memory->readMem64(reg, addr))
                return {};

            return PWTS::RWData<PWTS::Intel::MCHBARPkgRaplLimit>({
                .pl1 = static_cast<int>(getBitfield(14, 0, reg) * skuUnit.powerUnit * 1000),
                .pl2 = static_cast<int>(getBitfield(46, 32, reg) * skuUnit.powerUnit * 1000),
                .pl1Time = static_cast<int>(std::pow(2, getBitfield(21, 17, reg)) * (1.f + static_cast<float>(getBitfield(23, 22, reg))/4.f) * skuUnit.timeUnit * 1000),
                .pl1Enable = getBitfield(15, 15, reg) == 1,
                .pl2Enable = getBitfield(47, 47, reg) == 1,
                .lock = getBitfield(63, 63, reg) == 1
            }, true);
        }

        [[nodiscard]]
        bool setPkgRaplLimit(const PWTS::RWData<PWTS::Intel::MCHBARPkgRaplLimit> &data, const PWTS::ROData<MCHBAR_PACKAGE_POWER_SKU_UNIT::PkgPowerSKUUnits> &powerSkuUnit) const override {
            if (!data.isValid())
                return true;

            const MCHBAR_PACKAGE_POWER_SKU_UNIT::PkgPowerSKUUnits skuUnit = powerSkuUnit.getValue();
            const PWTS::Intel::MCHBARPkgRaplLimit pkgPowerLim = data.getValue();
            const int pl1 = static_cast<uint64_t>(pkgPowerLim.pl1 / skuUnit.powerUnit / 1000);
            const int pl2 = static_cast<uint64_t>(pkgPowerLim.pl2 / skuUnit.powerUnit / 1000);
            PowerLimitRawTimeWindow timeWindow;
            uint64_t reg;

            if (!powerSkuUnit.isValid() || !memory->readMem64(reg, addr))
                return false;

            reg = setBitfield(14, 0, pl1, reg);
            reg = setBitfield(15, 15, pkgPowerLim.pl1Enable, reg);
            reg = setBitfield(46, 32, pl2, reg);
            reg = setBitfield(47, 47, pkgPowerLim.pl2Enable, reg);
            reg = setBitfield(63, 63, pkgPowerLim.lock, reg);

            timeWindow = getRawTimeWindow(static_cast<float>(pkgPowerLim.pl1Time) / 1000, skuUnit.timeUnit);
            if (timeWindow.y != -1) {
                reg = setBitfield(21, 17, timeWindow.y, reg);
                reg = setBitfield(23, 22, timeWindow.z, reg);
            }

            if (!memory->writeMem64(reg, addr) || !memory->readMem64(reg, addr))
                return false;

            return getBitfield(14, 0, reg) == pl1 &&
                getBitfield(15, 15, reg) == pkgPowerLim.pl1Enable &&
                getBitfield(46, 32, reg) == pl2 &&
                getBitfield(47, 47, reg) == pkgPowerLim.pl2Enable &&
                getBitfield(63, 63, reg) == pkgPowerLim.lock;
        }
    };
}
