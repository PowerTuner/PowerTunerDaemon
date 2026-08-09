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

namespace PWTD::AMD {
    class MSR_PSTATE_CONTROL final : public CPURegister {
    private:
        static constexpr uint32_t addr = 0xc0010062;

    public:
        [[nodiscard]]
        PWTS::RWData<int> getPStateControlData(const int cpu) const {
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, cpu))
                return {};

            return PWTS::RWData<int>(static_cast<int>(getBitfield(3, 0, reg)), true);
        }

        [[nodiscard]]
        bool setPStateControl(const int cpu, const PWTS::RWData<int> &data) const {
            if (!data.isValid())
                return true;

            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, cpu))
                return false;

            reg = setBitfield(3, 0, data.getValue(), reg);

            if (!msrUtils->writeMSR(reg, addr, cpu) || !msrUtils->readMSR(reg, addr, cpu))
                return false;

            return getBitfield(3, 0, reg) == data.getValue();
        }
    };
}
