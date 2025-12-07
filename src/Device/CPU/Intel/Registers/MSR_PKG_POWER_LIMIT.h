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
        struct pkgPowerLimit final {
            uint64_t pl1 :15; // 14:0
            uint64_t pl1Enable :1; // 15
            uint64_t pl1Clamp :1; // 16
            uint64_t pl1TimeY :5; // 21:17
            uint64_t pl1TimeZ :2; // 23:22
            // 31:24 reserved:8
            uint64_t pl2 :15; // 46:32
            uint64_t pl2Enable :1; // 47
            uint64_t pl2Clamp :1; // 48
            uint64_t pl2TimeY :5; // 53:49
            uint64_t pl2TimeZ :2; // 55:54
            // 62:56 reserved:7
            uint64_t lock :1; // 63
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, pkgPowerLimit &regVal) const {
            try {
                regVal.pl1 = getBitfield(14, 0, raw);
                regVal.pl1Enable = getBitfield(15, 15, raw);
                regVal.pl1Clamp = getBitfield(16, 16, raw);
                regVal.pl1TimeY = getBitfield(21, 17, raw);
                regVal.pl1TimeZ = getBitfield(23, 22, raw);
                regVal.pl2 = getBitfield(46, 32, raw);
                regVal.pl2Enable = getBitfield(47, 47, raw);
                regVal.pl2Clamp = getBitfield(48, 48, raw);
                regVal.pl2TimeY = getBitfield(53, 49, raw);
                regVal.pl2TimeZ = getBitfield(55, 54, raw);
                regVal.lock = getBitfield(63, 63, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const pkgPowerLimit &regVal, uint64_t &raw) const {
            try {
                setBitfield(14, 0, regVal.pl1, raw);
                setBitfield(15, 15, regVal.pl1Enable, raw);
                setBitfield(16, 16, regVal.pl1Clamp, raw);
                setBitfield(21, 17, regVal.pl1TimeY, raw);
                setBitfield(23, 22, regVal.pl1TimeZ, raw);
                setBitfield(46, 32, regVal.pl2, raw);
                setBitfield(47, 47, regVal.pl2Enable, raw);
                setBitfield(48, 48, regVal.pl2Clamp, raw);
                setBitfield(53, 49, regVal.pl2TimeY, raw);
                setBitfield(55, 54, regVal.pl2TimeZ, raw);
                setBitfield(63, 63, regVal.lock, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        MSR_PKG_POWER_LIMIT() {
            addr = 0x610;
        }

        PWTS::RWData<PWTS::Intel::PkgPowerLimit> getPkgPowerLimitData(const PWTS::ROData<MSR_RAPL_POWER_UNIT::RAPLPowerUnits> &powerUnits) const {
            const MSR_RAPL_POWER_UNIT::RAPLPowerUnits raplUnit = powerUnits.getValue();
            pkgPowerLimit regVal {};
            uint64_t raw = 0;

            if (!powerUnits.isValid() || !msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::PkgPowerLimit>({
                .pl1 = static_cast<int>(regVal.pl1 * raplUnit.powerUnit * 1000),
                .pl2 = static_cast<int>(regVal.pl2 * raplUnit.powerUnit * 1000),
                .pl1Time = static_cast<int>(qPow(2, regVal.pl1TimeY) * (1.f + static_cast<float>(regVal.pl1TimeZ)/4.f) * raplUnit.timeUnit * 1000),
                .pl2Time = static_cast<int>(qPow(2, regVal.pl2TimeY) * (1.f + static_cast<float>(regVal.pl2TimeZ)/4.f) * raplUnit.timeUnit * 1000),
                .pl1Clamp = regVal.pl1Clamp == 1,
                .pl2Clamp = regVal.pl2Clamp == 1,
                .pl1Enable = regVal.pl1Enable == 1,
                .pl2Enable = regVal.pl2Enable == 1,
                .lock = regVal.lock == 1,
            }, true);
        }

        [[nodiscard]]
        bool setPkgPowerLimit(const PWTS::RWData<PWTS::Intel::PkgPowerLimit> &data, const PWTS::ROData<MSR_RAPL_POWER_UNIT::RAPLPowerUnits> &powerUnits) const {
            if (!data.isValid())
                return true;

            const MSR_RAPL_POWER_UNIT::RAPLPowerUnits powUnits = powerUnits.getValue();
            const PWTS::Intel::PkgPowerLimit pkgPowerLim = data.getValue();
            PowerLimitRawTimeWindow timeWindow;
            pkgPowerLimit regVal {};
            uint64_t raw = 0, cur = 0;

            if (!powerUnits.isValid() || !msrUtils->readMSR(raw, addr, 0))
                return false;

            regVal.pl1 = static_cast<uint64_t>(pkgPowerLim.pl1 / powUnits.powerUnit / 1000);
            regVal.pl1Enable = pkgPowerLim.pl1Enable;
            regVal.pl1Clamp = pkgPowerLim.pl1Clamp;
            regVal.pl2 = static_cast<uint64_t>(pkgPowerLim.pl2 / powUnits.powerUnit / 1000);
            regVal.pl2Enable = pkgPowerLim.pl2Enable;
            regVal.pl2Clamp = pkgPowerLim.pl2Clamp;
            regVal.lock = pkgPowerLim.lock;

            timeWindow = getRawTimeWindow(static_cast<float>(pkgPowerLim.pl1Time) / 1000, powUnits.timeUnit);
            if (timeWindow.y != -1) {
                regVal.pl1TimeY = timeWindow.y;
                regVal.pl1TimeZ = timeWindow.z;
            }

            timeWindow = getRawTimeWindow(static_cast<float>(pkgPowerLim.pl2Time) / 1000, powUnits.timeUnit);
            if (timeWindow.y != -1) {
                regVal.pl2TimeY = timeWindow.y;
                regVal.pl2TimeZ = timeWindow.z;
            }

            if (!setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
