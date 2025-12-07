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

#include "../../CPURegister.h"
#include "../../Utils/CPUUtils.h"
#include "pwtShared/Include/CPU/Intel/MiscProcFeatures.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class IA32_MISC_ENABLE final: public CPURegister {
    private:
        struct ia32MiscEnable final {
            uint64_t fastStringsEnable :1; // 0
            // 6:1 reserved:6
            uint64_t performanceMonitoringAvailable :1; // 7
            // 10:8 reserved:3
            uint64_t branchTraceStorageUnavailable :1; // 11
            uint64_t processorEventBasedSampling :1; // 12
            // 15:13 reserved:3
            uint64_t enhancedIntelSpeedStepTechnology :1; // 16
            // 17 reserved:1
            uint64_t enableMonitorFsm :1; // 18
            // 21:19 reserved:3
            uint64_t limitCpuidMaxVal :1; // 22
            uint64_t xtrpMessageDisable :1; // 23
            // 33:24 reserved:10
            uint64_t xdBitDisable :1; // 34
            // 37:35 reserved:3
            uint64_t turboModeDisable :1; // 38
            // 63:39 reserved:25
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, ia32MiscEnable &regVal) const {
            try {
                regVal.fastStringsEnable = getBitfield(0, 0, raw);
                regVal.performanceMonitoringAvailable = getBitfield(7, 7, raw);
                regVal.branchTraceStorageUnavailable = getBitfield(11, 11, raw);
                regVal.processorEventBasedSampling = getBitfield(12, 12, raw);
                regVal.enhancedIntelSpeedStepTechnology = getBitfield(16, 16, raw);
                regVal.enableMonitorFsm = getBitfield(18, 18, raw);
                regVal.limitCpuidMaxVal = getBitfield(22, 22, raw);
                regVal.xtrpMessageDisable = getBitfield(23, 23, raw);
                regVal.xdBitDisable = getBitfield(34, 34, raw);
                regVal.turboModeDisable = getBitfield(38, 38, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const ia32MiscEnable &regVal, uint64_t &raw) const {
            try {
                setBitfield(16, 16, regVal.enhancedIntelSpeedStepTechnology, raw);
                setBitfield(38, 38, regVal.turboModeDisable, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        IA32_MISC_ENABLE() {
            addr = 0x1a0;
        }

        PWTS::RWData<PWTS::Intel::MiscProcFeatures> getMiscProcessorFeaturesData() const {
            ia32MiscEnable regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::MiscProcFeatures>({
                .enhancedSpeedStep = regVal.enhancedIntelSpeedStepTechnology == 1,
                .disableTurboMode = regVal.turboModeDisable == 1
            }, true);
        }

        [[nodiscard]]
        bool setMiscProcessorFeatures(const PWTS::RWData<PWTS::Intel::MiscProcFeatures> &data) const {
            if (!data.isValid())
                return true;

            const PWTS::Intel::MiscProcFeatures miscFeat = data.getValue();
            ia32MiscEnable regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.enhancedIntelSpeedStepTechnology = miscFeat.enhancedSpeedStep;
            regVal.turboModeDisable = miscFeat.disableTurboMode;

            if (!msrUtils->readMSR(raw, addr, 0) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
