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
#include "pwtShared/Include/Types/ROData.h"

namespace PWTD::Intel {
    class MSR_RAPL_POWER_UNIT final: public CPURegister {
    private:
        struct raplPowerUnit final {
            uint64_t powerUnits :4; // 3:0
            // 7:4 reserved:4
            uint64_t energyStatusUnits :5; // 12:8
            // 15:13 reserved:3
            uint64_t timeUnits :4; // 19:16
            // 63:20 reserved:44
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, raplPowerUnit &regVal) const {
            try {
                regVal.powerUnits = getBitfield(3, 0, raw);
                regVal.energyStatusUnits = getBitfield(7, 4, raw);
                regVal.timeUnits = getBitfield(19, 16, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        struct [[nodiscard]] RAPLPowerUnits final {
            double powerUnit;
            double timeUnit;
        };

        MSR_RAPL_POWER_UNIT() {
            addr = 0x606;
        }

        PWTS::ROData<RAPLPowerUnits> getPowerUnitData() const {
            raplPowerUnit regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::ROData<RAPLPowerUnits>({
                .powerUnit = 1 / qPow(2, regVal.powerUnits),
                .timeUnit = 1 / qPow(2, regVal.timeUnits)
            }, true);
        }
    };
}
