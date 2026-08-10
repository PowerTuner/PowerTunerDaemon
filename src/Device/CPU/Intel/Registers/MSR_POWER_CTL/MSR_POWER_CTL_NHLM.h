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
    class MSR_POWER_CTL_NHLM final: public MSR_POWER_CTL {
    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::PowerCtl> get() const override {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return PWTS::RWData<PWTS::Intel::PowerCtl>({
                .c1eEnable = getBitfield(1, 1, reg) == 1
            }, true);
        }

        [[nodiscard]]
        bool set(const PWTS::RWData<PWTS::Intel::PowerCtl> &data) const override {
            if (!data.isValid())
                return true;

            const PWTS::Intel::PowerCtl powCtl = data.getValue();
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(1, 1, powCtl.c1eEnable, reg);

            if (!msrDev->write(reg, addr, 0) || !msrDev->read(addr, 0, reg))
                return false;

            return getBitfield(1, 1, reg) == powCtl.c1eEnable;
        }
    };
}
