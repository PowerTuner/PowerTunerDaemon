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
    private:
        struct packageRaplLimit final {
            uint64_t packagePowerLimit1 :15; // 14:0
            uint64_t packagePowerLimit1Enable :1; // 15
            uint64_t packageClampingLimitation1 :1; // 16
            uint64_t packageLimitation1TimeWindowY :5; // 21:17
            uint64_t packageLimitation1TimeWindowX :2; // 23:22
            // 31:24 reserved:8
            uint64_t packagePowerLimit2 :15; // 46:32
            uint64_t packagePowerLimit2Enable :1; // 47
            // 62:48 reserved:15
            uint64_t lock :1; // 63
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, packageRaplLimit &regVal) const {
            try {
                regVal.packagePowerLimit1 = getBitfield(14, 0, raw);
                regVal.packagePowerLimit1Enable = getBitfield(15, 15, raw);
                regVal.packageClampingLimitation1 = getBitfield(16, 16, raw);
                regVal.packageLimitation1TimeWindowY = getBitfield(21, 17, raw);
                regVal.packageLimitation1TimeWindowX = getBitfield(23, 22, raw);
                regVal.packagePowerLimit2 = getBitfield(46, 32, raw);
                regVal.packagePowerLimit2Enable = getBitfield(47, 47, raw);
                regVal.lock = getBitfield(63, 63, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const packageRaplLimit &regVal, uint64_t &raw) const {
            try {
                setBitfield(14, 0, regVal.packagePowerLimit1, raw);
                setBitfield(15, 15, regVal.packagePowerLimit1Enable, raw);
                setBitfield(16, 16, regVal.packageClampingLimitation1, raw);
                setBitfield(21, 17, regVal.packageLimitation1TimeWindowY, raw);
                setBitfield(23, 22, regVal.packageLimitation1TimeWindowX, raw);
                setBitfield(46, 32, regVal.packagePowerLimit2, raw);
                setBitfield(47, 47, regVal.packagePowerLimit2Enable, raw);
                setBitfield(63, 63, regVal.lock, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        explicit MCHBAR_PACKAGE_RAPL_LIMIT_TGL(const uint32_t base): MCHBAR_PACKAGE_RAPL_LIMIT(base) {}

        PWTS::RWData<PWTS::Intel::MCHBARPkgRaplLimit> getPkgRaplLimitData(const PWTS::ROData<MCHBAR_PACKAGE_POWER_SKU_UNIT::PkgPowerSKUUnits> &powerSkuUnit) const override {
            const MCHBAR_PACKAGE_POWER_SKU_UNIT::PkgPowerSKUUnits skuUnit = powerSkuUnit.getValue();
            packageRaplLimit regVal {};
            uint64_t raw = 0;

            if (!powerSkuUnit.isValid() || !memory->readMem64(raw, addr) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::MCHBARPkgRaplLimit>({
                .pl1 = static_cast<int>(regVal.packagePowerLimit1 * skuUnit.powerUnit * 1000),
                .pl2 = static_cast<int>(regVal.packagePowerLimit2 * skuUnit.powerUnit * 1000),
                .pl1Time = static_cast<int>(qPow(2, regVal.packageLimitation1TimeWindowY) * (1.f + static_cast<float>(regVal.packageLimitation1TimeWindowX)/4.f) * skuUnit.timeUnit * 1000),
                .pl1Clamp = regVal.packageClampingLimitation1 == 1,
                .pl1Enable = regVal.packagePowerLimit1Enable == 1,
                .pl2Enable = regVal.packagePowerLimit2Enable == 1,
                .lock = regVal.lock == 1
            }, true);
        }

        [[nodiscard]]
        bool setPkgRaplLimit(const PWTS::RWData<PWTS::Intel::MCHBARPkgRaplLimit> &data, const PWTS::ROData<MCHBAR_PACKAGE_POWER_SKU_UNIT::PkgPowerSKUUnits> &powerSkuUnit) const override {
            if (!data.isValid())
                return true;

            const MCHBAR_PACKAGE_POWER_SKU_UNIT::PkgPowerSKUUnits skuUnit = powerSkuUnit.getValue();
            const PWTS::Intel::MCHBARPkgRaplLimit pkgPowerLim = data.getValue();
            PowerLimitRawTimeWindow timeWindow;
            packageRaplLimit regVal {};
            uint64_t raw = 0, cur = 0;

            if (!powerSkuUnit.isValid() || !memory->readMem64(raw, addr))
                return false;

            regVal.packagePowerLimit1 = static_cast<uint64_t>(pkgPowerLim.pl1 / skuUnit.powerUnit / 1000);
            regVal.packagePowerLimit1Enable = pkgPowerLim.pl1Enable;
            regVal.packageClampingLimitation1 = pkgPowerLim.pl1Clamp;
            regVal.packagePowerLimit2 = static_cast<uint64_t>(pkgPowerLim.pl2 / skuUnit.powerUnit / 1000);
            regVal.packagePowerLimit2Enable = pkgPowerLim.pl2Enable;
            regVal.lock = pkgPowerLim.lock;

            timeWindow = getRawTimeWindow(static_cast<float>(pkgPowerLim.pl1Time) / 1000, skuUnit.timeUnit);
            if (timeWindow.y != -1) {
                regVal.packageLimitation1TimeWindowY = timeWindow.y;
                regVal.packageLimitation1TimeWindowX = timeWindow.z;
            }

            if (!setRawValue(regVal, raw) || !memory->writeMem64(raw, addr) || !memory->readMem64(cur, addr))
                return false;

            return cur == raw;
        }
    };
}
