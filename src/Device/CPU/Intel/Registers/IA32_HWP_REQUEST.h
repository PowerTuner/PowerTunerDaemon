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
#include "../Utils/IntelRegisterUtils.h"
#include "pwtShared/Include/CPU/Intel/HWPRequest.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class IA32_HWP_REQUEST final: public CPURegister {
    private:
        struct ia32HWPRequest final {
            uint64_t minimumPerformance :8; // 7:0
            uint64_t maximumPeformance :8; // 15:8
            uint64_t desiredPerformance :8; // 23:16
            uint64_t energyPerformancePreference :8; // 31:24
            uint64_t activityWindowMantissa :7; // 38:32
            uint64_t activityWindowExponent :3; // 41:39
            uint64_t packageControl :1; // 42
            // 58:43 reserved:16
            uint64_t activityWindowValid :1; // 59
            uint64_t eppValid :1; // 60
            uint64_t desiredValid :1; // 61
            uint64_t maximumValid :1; // 62
            uint64_t minimumValid :1; // 63
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, ia32HWPRequest &regVal) const {
            try {
                regVal.minimumPerformance = getBitfield(7, 0, raw);
                regVal.maximumPeformance = getBitfield(15, 8, raw);
                regVal.desiredPerformance = getBitfield(23, 16, raw);
                regVal.energyPerformancePreference = getBitfield(31, 24, raw);
                regVal.activityWindowMantissa = getBitfield(38, 32, raw);
                regVal.activityWindowExponent = getBitfield(41, 39, raw);
                regVal.packageControl = getBitfield(42, 42, raw);
                regVal.activityWindowValid = getBitfield(59, 59, raw);
                regVal.eppValid = getBitfield(60, 60, raw);
                regVal.desiredValid = getBitfield(61, 61, raw);
                regVal.maximumValid = getBitfield(62, 62, raw);
                regVal.minimumValid = getBitfield(63, 63, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const ia32HWPRequest &regVal, uint64_t &raw) const {
            try {
                setBitfield(7, 0, regVal.minimumPerformance, raw);
                setBitfield(15, 8, regVal.maximumPeformance, raw);
                setBitfield(23, 16, regVal.desiredPerformance, raw);
                setBitfield(31, 24, regVal.energyPerformancePreference, raw);
                setBitfield(38, 32, regVal.activityWindowMantissa, raw);
                setBitfield(41, 39, regVal.activityWindowExponent, raw);
                setBitfield(42, 42, regVal.packageControl, raw);
                setBitfield(59, 59, regVal.activityWindowValid, raw);
                setBitfield(60, 60, regVal.eppValid, raw);
                setBitfield(61, 61, regVal.desiredValid, raw);
                setBitfield(62, 62, regVal.maximumValid, raw);
                setBitfield(63, 63, regVal.minimumValid, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        IA32_HWP_REQUEST()  {
            addr = 0x774;
        }

        PWTS::RWData<PWTS::Intel::HWPRequest> getHWPRequestData(const int cpu) const {
            ia32HWPRequest regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, cpu) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::HWPRequest>({
                .requestPkg = {
                    .min = static_cast<int>(regVal.minimumPerformance),
                    .max = static_cast<int>(regVal.maximumPeformance),
                    .desired = static_cast<int>(regVal.desiredPerformance),
                    .epp = static_cast<int>(regVal.energyPerformancePreference),
                    .acw = static_cast<int>(regVal.activityWindowMantissa * qPow(10, regVal.activityWindowExponent)),//todo check
                },
                .packageControl = regVal.packageControl == 1,
                .acwValid = regVal.activityWindowValid == 1,
                .eppValid = regVal.eppValid == 1,
                .desiredValid = regVal.desiredValid == 1,
                .maxValid = regVal.maximumValid == 1,
                .minValid = regVal.minimumValid == 1
            }, true);
        }

        [[nodiscard]]
        bool setHWPRequest(const int cpu, const PWTS::RWData<PWTS::Intel::HWPRequest> &data) const {
            if (!data.isValid())
                return true;

            const PWTS::Intel::HWPRequest hwpReq = data.getValue();
            const HWPActivityWindowBits acwBits = getHWPActivityWindowBitsFromMicroSecond(hwpReq.requestPkg.acw);
            ia32HWPRequest regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.minimumPerformance = hwpReq.requestPkg.min;
            regVal.maximumPeformance = hwpReq.requestPkg.max;
            regVal.desiredPerformance = hwpReq.requestPkg.desired;
            regVal.energyPerformancePreference = hwpReq.requestPkg.epp;
            regVal.activityWindowMantissa = acwBits.mantissa;
            regVal.activityWindowExponent = acwBits.exponent;
            regVal.packageControl = hwpReq.packageControl;
            regVal.minimumValid = hwpReq.minValid;
            regVal.maximumValid = hwpReq.maxValid;
            regVal.desiredValid = hwpReq.desiredValid;
            regVal.eppValid = hwpReq.eppValid;
            regVal.activityWindowValid = hwpReq.acwValid;

            if (!msrUtils->readMSR(raw, addr, cpu) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, cpu) || !msrUtils->readMSR(cur, addr, cpu))
                return false;

            return cur == raw;
        }
    };
}
