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

#include <stdexcept>

#include "MSR_POWER_CTL.h"
#include "../../../Utils/CPUUtils.h"

namespace PWTD::Intel {
    class MSR_POWER_CTL_CU1 final: public MSR_POWER_CTL {
    private:
        struct powerCtl final {
            uint64_t bdProcHot :1; // 0
            uint64_t c1eEnable :1; // 1
            uint64_t sapmImcC2Policy :1; // 2
            uint64_t fastBrkSnpEn :1; // 3
            // 17:4 reserved:14
            uint64_t powerPerformancePlatformOverride :1; // 18
            uint64_t disableEnergyEfficiencyOptimization :1; // 19
            uint64_t disableRaceToHaltOptimization :1; // 20
            uint64_t prochotOutputDisable :1; // 21
            uint64_t prochotConfigurableResponseEnable :1; // 22
            uint64_t vrThermAlertDisableLock :1; // 23
            uint64_t vrThermAlertDisable :1; // 24
            uint64_t ringEEDisable :1; // 25
            uint64_t saOptimizationDisable :1; // 26
            uint64_t ookDisable :1; // 27
            uint64_t hwpAutonomousDisable :1; // 28
            // 29 reserved:1
            uint64_t cstatePrewakeDisable :1; // 30
            // 63:31 reserved:33
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, powerCtl &regVal) const {
            try {
                regVal.bdProcHot = getBitfield(0, 0, raw);
                regVal.c1eEnable = getBitfield(1, 1, raw);
                regVal.sapmImcC2Policy = getBitfield(2, 2, raw);
                regVal.fastBrkSnpEn = getBitfield(3, 3, raw);
                regVal.powerPerformancePlatformOverride = getBitfield(18, 18, raw);
                regVal.disableEnergyEfficiencyOptimization = getBitfield(19, 19, raw);
                regVal.disableRaceToHaltOptimization = getBitfield(20, 20, raw);
                regVal.prochotOutputDisable = getBitfield(21, 21, raw);
                regVal.prochotConfigurableResponseEnable = getBitfield(22, 22, raw);
                regVal.vrThermAlertDisableLock = getBitfield(23, 23, raw);
                regVal.vrThermAlertDisable = getBitfield(24, 24, raw);
                regVal.ringEEDisable = getBitfield(25, 25, raw);
                regVal.saOptimizationDisable = getBitfield(26, 26, raw);
                regVal.ookDisable = getBitfield(27, 27, raw);
                regVal.hwpAutonomousDisable = getBitfield(28, 28, raw);
                regVal.cstatePrewakeDisable = getBitfield(30, 30, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const powerCtl &regVal, uint64_t &raw) const {
            try {
                setBitfield(0, 0, regVal.bdProcHot, raw);
                setBitfield(1, 1, regVal.c1eEnable, raw);
                setBitfield(2, 2, regVal.sapmImcC2Policy, raw);
                setBitfield(3, 3, regVal.fastBrkSnpEn, raw);
                setBitfield(18, 18, regVal.powerPerformancePlatformOverride, raw);
                setBitfield(19, 19, regVal.disableEnergyEfficiencyOptimization, raw);
                setBitfield(20, 20, regVal.disableRaceToHaltOptimization, raw);
                setBitfield(21, 21, regVal.prochotOutputDisable, raw);
                setBitfield(22, 22, regVal.prochotConfigurableResponseEnable, raw);
                setBitfield(23, 23, regVal.vrThermAlertDisableLock, raw);
                setBitfield(24, 24, regVal.vrThermAlertDisable, raw);
                setBitfield(25, 25, regVal.ringEEDisable, raw);
                setBitfield(26, 26, regVal.saOptimizationDisable, raw);
                setBitfield(27, 27, regVal.ookDisable, raw);
                setBitfield(28, 28, regVal.hwpAutonomousDisable, raw);
                setBitfield(30, 30, regVal.cstatePrewakeDisable, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        PWTS::RWData<PWTS::Intel::PowerCtl> getPowerCtlData() const override {
            powerCtl regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::PowerCtl>({
                .bdProcHot = regVal.bdProcHot == 1,
                .c1eEnable = regVal.c1eEnable == 1,
                .sapmImcC2Policy = regVal.sapmImcC2Policy == 1,
                .fastBrkSnpEn = regVal.fastBrkSnpEn == 1,
                .powerPerformancePlatformOverride = regVal.powerPerformancePlatformOverride == 1,
                .disableEnergyEfficiencyOpt = regVal.disableEnergyEfficiencyOptimization == 1,
                .disableRaceToHaltOpt = regVal.disableRaceToHaltOptimization == 1,
                .prochotOutputDisable = regVal.prochotOutputDisable == 1,
                .prochotConfigurableResponseEnable = regVal.prochotConfigurableResponseEnable == 1,
                .vrThermAlertDisableLock = regVal.vrThermAlertDisableLock == 1,
                .vrThermAlertDisable = regVal.vrThermAlertDisable == 1,
                .ringEEDisable = regVal.ringEEDisable == 1,
                .saOptimizationDisable = regVal.saOptimizationDisable == 1,
                .ookDisable = regVal.ookDisable == 1,
                .hwpAutonomousDisable = regVal.hwpAutonomousDisable == 1,
                .cstatePrewakeDisable = regVal.cstatePrewakeDisable == 1
            }, true);
        }

        [[nodiscard]]
        bool setPowerCtl(const PWTS::RWData<PWTS::Intel::PowerCtl> &data) const override {
            if (!data.isValid())
                return true;

            const PWTS::Intel::PowerCtl powCtl = data.getValue();
            powerCtl regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.bdProcHot = powCtl.bdProcHot;
            regVal.c1eEnable = powCtl.c1eEnable;
            regVal.sapmImcC2Policy = powCtl.sapmImcC2Policy;
            regVal.fastBrkSnpEn = powCtl.fastBrkSnpEn;
            regVal.powerPerformancePlatformOverride = powCtl.powerPerformancePlatformOverride;
            regVal.disableEnergyEfficiencyOptimization = powCtl.disableEnergyEfficiencyOpt;
            regVal.disableRaceToHaltOptimization = powCtl.disableRaceToHaltOpt;
            regVal.prochotOutputDisable = powCtl.prochotOutputDisable;
            regVal.prochotConfigurableResponseEnable = powCtl.prochotConfigurableResponseEnable;
            regVal.vrThermAlertDisableLock = powCtl.vrThermAlertDisableLock;
            regVal.vrThermAlertDisable = powCtl.vrThermAlertDisable;
            regVal.ringEEDisable = powCtl.ringEEDisable;
            regVal.saOptimizationDisable = powCtl.saOptimizationDisable;
            regVal.ookDisable = powCtl.ookDisable;
            regVal.hwpAutonomousDisable = powCtl.hwpAutonomousDisable;
            regVal.cstatePrewakeDisable = powCtl.cstatePrewakeDisable;

            if (!msrUtils->readMSR(raw, addr, 0) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
