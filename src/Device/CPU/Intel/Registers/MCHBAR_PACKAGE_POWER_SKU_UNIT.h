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

#include "../MCHBAR/MCHBARRegister.h"
#include "../../Utils/CPUUtils.h"
#include "pwtShared/Include/Types/ROData.h"

namespace PWTD::Intel {
    class MCHBAR_PACKAGE_POWER_SKU_UNIT final: public MCHBARRegister {
    private:
        struct packagePowerSkuUnit final {
            uint32_t powerUnit :4; // 3:0
            // 7:4 reserved:4
            uint32_t energyUnit :5; // 12:8
            // 15:13 reserved:3
            uint32_t timeUnit :4; // 19:16
            // 31:20 reserved:12
        };

        [[nodiscard]]
        bool setBitfields(const uint32_t raw, packagePowerSkuUnit &regVal) const {
            try {
                regVal.powerUnit = getBitfield(3, 0, raw);
                regVal.energyUnit = getBitfield(12, 8, raw);
                regVal.timeUnit = getBitfield(19, 16, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        struct [[nodiscard]] PkgPowerSKUUnits final {
            double powerUnit;
            double timeUnit;
        };

        explicit MCHBAR_PACKAGE_POWER_SKU_UNIT(const uint32_t base): MCHBARRegister(base, 0x5938) {}

        PWTS::ROData<PkgPowerSKUUnits> getPkgSKUPowerUnitData() const {
            packagePowerSkuUnit regVal {};
            uint64_t raw = 0;

            if (!memory->readMem32(raw, addr) || !setBitfields(raw, regVal))
                return {};

            return PWTS::ROData<PkgPowerSKUUnits>({
                .powerUnit = 1 / qPow(2, regVal.powerUnit),
                .timeUnit = 1 / qPow(2, regVal.timeUnit)
            }, true);
        }
    };
}
