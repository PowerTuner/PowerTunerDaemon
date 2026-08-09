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
#include "MSR_TEMPERATURE_TARGET.h"
#include "../../../Utils/Utils.h"

namespace PWTD::Intel {
    class MSR_TEMPERATURE_TARGET_NHLM final: public MSR_TEMPERATURE_TARGET {
    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::TemperatureTarget> getTemperatureTargetData() const override {
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, 0))
                return {};

            return PWTS::RWData<PWTS::Intel::TemperatureTarget>({
                .temperatureTarget = static_cast<int>(getBitfield(23, 16, reg))
            }, true);
        }
    };
}
