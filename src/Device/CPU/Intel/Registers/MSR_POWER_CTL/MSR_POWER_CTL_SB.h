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
#include "MSR_POWER_CTL.h"
#include "../../../Utils/Utils.h"

namespace PWTD::Intel {
    class MSR_POWER_CTL_SB final: public MSR_POWER_CTL {
    public:
        [[nodiscard]]
        std::optional<LIBPWT::Intel::PowerCtl> get() const override {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return LIBPWT::Intel::PowerCtl {
                .bdProcHot = getBitfield(0, 0, reg) == 1,
                .c1eEnable = getBitfield(1, 1, reg) == 1,
                .disableEnergyEfficiencyOpt = getBitfield(19, 19, reg) == 1,
                .disableRaceToHaltOpt = getBitfield(20, 20, reg) == 1
            };
        }

        [[nodiscard]]
        bool set(const LIBPWT::Intel::PowerCtl &data) const override {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(0, 0, data.bdProcHot, reg);
            reg = setBitfield(1, 1, data.c1eEnable, reg);
            reg = setBitfield(19, 19, data.disableEnergyEfficiencyOpt, reg);
            reg = setBitfield(20, 20, data.disableRaceToHaltOpt, reg);

            return msrDev->write(reg, addr, 0) &&
                msrDev->read(addr, 0, reg) &&
                getBitfield(0, 0, reg) == data.bdProcHot &&
                getBitfield(1, 1, reg) == data.c1eEnable &&
                getBitfield(19, 19, reg) == data.disableEnergyEfficiencyOpt &&
                getBitfield(20, 20, reg) == data.disableRaceToHaltOpt;
        }
    };
}
