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

#include "MSR_TEMPERATURE_TARGET.h"
#include "../../../Utils/CPUUtils.h"

namespace PWTD::Intel {
    class MSR_TEMPERATURE_TARGET_NHLM final: public MSR_TEMPERATURE_TARGET {
    private:
        struct temperatureTarget final {
            // 15:0 reserved:16
            uint64_t tempTarget :8; // 23:16
            // 63:24 reserved:40
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, temperatureTarget &regVal) const {
            try {
                regVal.tempTarget = getBitfield(23, 16, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        PWTS::RWData<PWTS::Intel::TemperatureTarget> getTemperatureTargetData() const override {
            temperatureTarget regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::TemperatureTarget>({
                .temperatureTarget = static_cast<int>(regVal.tempTarget)
            }, true);
        }
    };
}
