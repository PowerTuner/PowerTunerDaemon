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
#include "../Utils/IntelRegisterUtils.h"
#include "../../Utils/CPUUtils.h"
#include "pwtShared/Include/CPU/Intel/HWPRequestPkg.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class IA32_HWP_REQUEST_PKG final: public CPURegister {
    private:
        struct ia32HWPRequestPkg final {
            uint64_t minimumPerformance :8; // 7:0
            uint64_t maximumPerformance :8; // 15:8
            uint64_t desiredPerformance :8; // 23:16
            uint64_t energyPerformancePreference :8; // 31:24
            uint64_t activityWindowMantissa :7; // 38:32
            uint64_t activityWindowExponent :3; // 41:39
            // 63:42 reserved:22
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, ia32HWPRequestPkg &regVal) const {
            try {
                regVal.minimumPerformance = getBitfield(7, 0, raw);
                regVal.maximumPerformance = getBitfield(15, 8, raw);
                regVal.desiredPerformance = getBitfield(23, 16, raw);
                regVal.energyPerformancePreference = getBitfield(31, 24, raw);
                regVal.activityWindowMantissa = getBitfield(38, 32, raw);
                regVal.activityWindowExponent = getBitfield(41, 39, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const ia32HWPRequestPkg &regVal, uint64_t &raw) const {
            try {
                setBitfield(7, 0, regVal.minimumPerformance, raw);
                setBitfield(15, 8, regVal.maximumPerformance, raw);
                setBitfield(23, 16, regVal.desiredPerformance, raw);
                setBitfield(31, 24, regVal.energyPerformancePreference, raw);
                setBitfield(38, 32, regVal.activityWindowMantissa, raw);
                setBitfield(41, 39, regVal.activityWindowExponent, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        IA32_HWP_REQUEST_PKG() {
            addr = 0x772;
        }

        PWTS::RWData<PWTS::Intel::HWPRequestPkg> getHWPRequestPkgData() const {
            ia32HWPRequestPkg regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::HWPRequestPkg>({
                .min = static_cast<int>(regVal.minimumPerformance),
                .max = static_cast<int>(regVal.maximumPerformance),
                .desired = static_cast<int>(regVal.desiredPerformance),
                .epp = static_cast<int>(regVal.energyPerformancePreference),
                .acw = static_cast<int>(regVal.activityWindowMantissa * qPow(10, regVal.activityWindowExponent)) //todo check
            }, true);
        }

        [[nodiscard]]
        bool setHWPRequestPkg(const PWTS::RWData<PWTS::Intel::HWPRequestPkg> &data) const {
            if (!data.isValid())
                return true;

            const PWTS::Intel::HWPRequestPkg hwpReq = data.getValue();
            const HWPActivityWindowBits acwBits = getHWPActivityWindowBitsFromMicroSecond(hwpReq.acw);
            ia32HWPRequestPkg regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.minimumPerformance = hwpReq.min;
            regVal.maximumPerformance = hwpReq.max;
            regVal.desiredPerformance = hwpReq.desired;
            regVal.energyPerformancePreference = hwpReq.epp;
            regVal.activityWindowMantissa = acwBits.mantissa;
            regVal.activityWindowExponent = acwBits.exponent;

            if (!msrUtils->readMSR(raw, addr, 0) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
