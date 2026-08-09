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
#include "../Utils/IntelRegisterUtils.h"

namespace PWTD::Intel {
    class MSR_PKG_POWER_LIMIT final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x610;

    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::PkgPowerLimit> getPkgPowerLimitData(const PWTS::ROData<MSR_RAPL_POWER_UNIT::RAPLPowerUnits> &powerUnits) const {
            if (!powerUnits.isValid())
                return {};

            const MSR_RAPL_POWER_UNIT::RAPLPowerUnits raplUnit = powerUnits.getValue();
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, 0))
                return {};

            return PWTS::RWData<PWTS::Intel::PkgPowerLimit>({
                .pl1 = static_cast<int>(getBitfield(14, 0, reg) * raplUnit.powerUnit * 1000),
                .pl2 = static_cast<int>(getBitfield(46, 32, reg) * raplUnit.powerUnit * 1000),
                .pl1Time = static_cast<int>(std::pow(2, getBitfield(21, 17, reg)) * (1.f + static_cast<float>(getBitfield(23, 22, reg)) / 4.f) * raplUnit.timeUnit * 1000),
                .pl2Time = static_cast<int>(std::pow(2, getBitfield(53, 49, reg)) * (1.f + static_cast<float>(getBitfield(55, 54, reg)) / 4.f) * raplUnit.timeUnit * 1000),
                .pl1Clamp = getBitfield(16, 16, reg) == 1,
                .pl2Clamp = getBitfield(48, 48, reg) == 1,
                .pl1Enable = getBitfield(15, 15, reg) == 1,
                .pl2Enable = getBitfield(47, 47, reg) == 1,
                .lock = getBitfield(63, 63, reg) == 1
            }, true);
        }

        [[nodiscard]]
        bool setPkgPowerLimit(const PWTS::RWData<PWTS::Intel::PkgPowerLimit> &data, const PWTS::ROData<MSR_RAPL_POWER_UNIT::RAPLPowerUnits> &powerUnits) const {
            if (!data.isValid())
                return true;

            if (!powerUnits.isValid())
                return false;

            const MSR_RAPL_POWER_UNIT::RAPLPowerUnits powUnits = powerUnits.getValue();
            const PWTS::Intel::PkgPowerLimit pkgPowerLim = data.getValue();
            PowerLimitRawTimeWindow timeWindow;
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, 0))
                return false;

            reg = setBitfield(14, 0, static_cast<uint64_t>(pkgPowerLim.pl1 / powUnits.powerUnit / 1000), reg);
            reg = setBitfield(15, 15, pkgPowerLim.pl1Enable, reg);
            reg = setBitfield(16, 16, pkgPowerLim.pl1Clamp, reg);
            reg = setBitfield(46, 32, static_cast<uint64_t>(pkgPowerLim.pl2 / powUnits.powerUnit / 1000), reg);
            reg = setBitfield(47, 47, pkgPowerLim.pl2Enable, reg);
            reg = setBitfield(48, 48, pkgPowerLim.pl2Clamp, reg);
            reg = setBitfield(63, 63, pkgPowerLim.lock, reg);

            timeWindow = getRawTimeWindow(static_cast<float>(pkgPowerLim.pl1Time) / 1000, powUnits.timeUnit);
            if (timeWindow.y != -1) {
                reg = setBitfield(21, 17, timeWindow.y, reg);
                reg = setBitfield(23, 22, timeWindow.z, reg);
            }

            timeWindow = getRawTimeWindow(static_cast<float>(pkgPowerLim.pl2Time) / 1000, powUnits.timeUnit);
            if (timeWindow.y != -1) {
                reg = setBitfield(53, 49, timeWindow.y, reg);
                reg = setBitfield(55, 54, timeWindow.z, reg);
            }

            if (!msrUtils->writeMSR(reg, addr, 0) || !msrUtils->readMSR(reg, addr, 0))
                return false;

            return getBitfield(14, 0, reg) == static_cast<uint64_t>(pkgPowerLim.pl1 / powUnits.powerUnit / 1000) &&
                getBitfield(15, 15, reg) == pkgPowerLim.pl1Enable &&
                getBitfield(16, 16, reg) == pkgPowerLim.pl1Clamp &&
                getBitfield(46, 32, reg) == static_cast<uint64_t>(pkgPowerLim.pl2 / powUnits.powerUnit / 1000) &&
                getBitfield(47, 47, reg) == pkgPowerLim.pl2Enable &&
                getBitfield(48, 48, reg) == pkgPowerLim.pl2Clamp &&
                getBitfield(63, 63, reg) == pkgPowerLim.lock;
        }
    };
}
