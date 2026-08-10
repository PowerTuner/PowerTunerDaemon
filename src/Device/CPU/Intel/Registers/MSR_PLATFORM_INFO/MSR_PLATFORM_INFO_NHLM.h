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
#include "MSR_PLATFORM_INFO.h"
#include "../../../Utils/Utils.h"

namespace PWTD::Intel {
    class MSR_PLATFORM_INFO_NHLM final: public MSR_PLATFORM_INFO {
    public:
        [[nodiscard]]
        std::optional<Info> get() const override {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return Info {
                .programmableRatioLimitForTurboMode = getBitfield(28, 28, reg) == 1,
                .programmableTDPLimitForTurboMode = getBitfield(29, 29, reg) == 1
            };
        }
    };
}
