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
#include "MSR_POWER_CTL.h"
#include "../../../Utils/Utils.h"

namespace PWTD::Intel {
    class MSR_POWER_CTL_CU1 final: public MSR_POWER_CTL {
    public:
        [[nodiscard]]
        std::optional<LIBPWT::Intel::PowerCtl> get() const override {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return LIBPWT::Intel::PowerCtl {
                .bdProcHot = getBitfield(0, 0, reg) == 1,
                .c1eEnable = getBitfield(1, 1, reg) == 1,
                .sapmImcC2Policy = getBitfield(2, 2, reg) == 1,
                .fastBrkSnpEn = getBitfield(3, 3, reg) == 1,
                .powerPerformancePlatformOverride = getBitfield(18, 18, reg) == 1,
                .disableEnergyEfficiencyOpt = getBitfield(19, 19, reg) == 1,
                .disableRaceToHaltOpt = getBitfield(20, 20, reg) == 1,
                .prochotOutputDisable = getBitfield(21, 21, reg) == 1,
                .prochotConfigurableResponseEnable = getBitfield(22, 22, reg) == 1,
                .vrThermAlertDisableLock = getBitfield(23, 23, reg) == 1,
                .vrThermAlertDisable = getBitfield(24, 24, reg) == 1,
                .ringEEDisable = getBitfield(25, 25, reg) == 1,
                .saOptimizationDisable = getBitfield(26, 26, reg) == 1,
                .ookDisable = getBitfield(27, 27, reg) == 1,
                .hwpAutonomousDisable = getBitfield(28, 28, reg) == 1,
                .cstatePrewakeDisable = getBitfield(30, 30, reg) == 1
            };
        }

        [[nodiscard]]
        bool set(const LIBPWT::Intel::PowerCtl &data) const override {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(0, 0, data.bdProcHot, reg);
            reg = setBitfield(1, 1, data.c1eEnable, reg);
            reg = setBitfield(2, 2, data.sapmImcC2Policy, reg);
            reg = setBitfield(3, 3, data.fastBrkSnpEn, reg);
            reg = setBitfield(18, 18, data.powerPerformancePlatformOverride, reg);
            reg = setBitfield(19, 19, data.disableEnergyEfficiencyOpt, reg);
            reg = setBitfield(20, 20, data.disableRaceToHaltOpt, reg);
            reg = setBitfield(21, 21, data.prochotOutputDisable, reg);
            reg = setBitfield(22, 22, data.prochotConfigurableResponseEnable, reg);
            reg = setBitfield(23, 23, data.vrThermAlertDisableLock, reg);
            reg = setBitfield(24, 24, data.vrThermAlertDisable, reg);
            reg = setBitfield(25, 25, data.ringEEDisable, reg);
            reg = setBitfield(26, 26, data.saOptimizationDisable, reg);
            reg = setBitfield(27, 27, data.ookDisable, reg);
            reg = setBitfield(28, 28, data.hwpAutonomousDisable, reg);
            reg = setBitfield(30, 30, data.cstatePrewakeDisable, reg);

            return msrDev->write(reg, addr, 0) &&
                msrDev->read(addr, 0, reg) &&
                getBitfield(0, 0, reg) == data.bdProcHot &&
                getBitfield(1, 1, reg) == data.c1eEnable &&
                getBitfield(2, 2, reg) == data.sapmImcC2Policy &&
                getBitfield(3, 3, reg) == data.fastBrkSnpEn &&
                getBitfield(18, 18, reg) == data.powerPerformancePlatformOverride &&
                getBitfield(19, 19, reg) == data.disableEnergyEfficiencyOpt &&
                getBitfield(20, 20, reg) == data.disableRaceToHaltOpt &&
                getBitfield(21, 21, reg) == data.prochotOutputDisable &&
                getBitfield(22, 22, reg) == data.prochotConfigurableResponseEnable &&
                getBitfield(23, 23, reg) == data.vrThermAlertDisableLock &&
                getBitfield(24, 24, reg) == data.vrThermAlertDisable &&
                getBitfield(25, 25, reg) == data.ringEEDisable &&
                getBitfield(26, 26, reg) == data.saOptimizationDisable &&
                getBitfield(27, 27, reg) == data.ookDisable &&
                getBitfield(28, 28, reg) == data.hwpAutonomousDisable &&
                getBitfield(30, 30, reg) == data.cstatePrewakeDisable;
        }
    };
}
