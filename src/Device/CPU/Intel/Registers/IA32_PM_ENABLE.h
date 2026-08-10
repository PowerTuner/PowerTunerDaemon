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
#include "../../CPURegister.h"
#include "../../../Utils/Utils.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class IA32_PM_ENABLE final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x770;

    public:
        [[nodiscard]]
        PWTS::RWData<int> get() const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return PWTS::RWData<int>(static_cast<int>(getBitfield(0, 0, reg)), true);
        }

        [[nodiscard]]
        bool set(const PWTS::RWData<int> &data) const {
            if (!data.isValid() || data.isIgnored())
                return true;

            const int enable = data.getValue();
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(0, 0, enable, reg);

            if (!msrDev->write(reg, addr, 0) || !msrDev->read(addr, 0, reg))
                return false;

            return getBitfield(0, 0, reg) == enable;
        }
    };
}
