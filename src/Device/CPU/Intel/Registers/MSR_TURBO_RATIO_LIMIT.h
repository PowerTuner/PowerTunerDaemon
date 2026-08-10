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
#include "pwtShared/Include/CPU/Intel/TurboRatioLimit.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class MSR_TURBO_RATIO_LIMIT final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x1ad;

    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::TurboRatioLimit> get() const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return PWTS::RWData<PWTS::Intel::TurboRatioLimit>({
                .maxRatioLimit1C = static_cast<int>(getBitfield(7, 0, reg)),
                .maxRatioLimit2C = static_cast<int>(getBitfield(15, 8, reg)),
                .maxRatioLimit3C = static_cast<int>(getBitfield(23, 16, reg)),
                .maxRatioLimit4C = static_cast<int>(getBitfield(31, 24, reg)),
                .maxRatioLimit5C = static_cast<int>(getBitfield(39, 32, reg)),
                .maxRatioLimit6C = static_cast<int>(getBitfield(47, 40, reg)),
                .maxRatioLimit7C = static_cast<int>(getBitfield(55, 48, reg)),
                .maxRatioLimit8C = static_cast<int>(getBitfield(63, 56, reg))
            }, true);
        }

        [[nodiscard]]
        bool set(const PWTS::RWData<PWTS::Intel::TurboRatioLimit> &data) const {
            if (!data.isValid())
                return true;

            const PWTS::Intel::TurboRatioLimit limit = data.getValue();
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(7, 0, limit.maxRatioLimit1C, reg);
            reg = setBitfield(15, 8, limit.maxRatioLimit2C, reg);
            reg = setBitfield(23, 16, limit.maxRatioLimit3C, reg);
            reg = setBitfield(31, 24, limit.maxRatioLimit4C, reg);
            reg = setBitfield(39, 32, limit.maxRatioLimit5C, reg);
            reg = setBitfield(47, 40, limit.maxRatioLimit6C, reg);
            reg = setBitfield(55, 48, limit.maxRatioLimit7C, reg);
            reg = setBitfield(63, 56, limit.maxRatioLimit8C, reg);

            if (!msrDev->write(reg, addr, 0) || !msrDev->read(addr, 0, reg))
                return false;

            return getBitfield(7, 0, reg) == limit.maxRatioLimit1C &&
                getBitfield(15, 8, reg) == limit.maxRatioLimit2C &&
                getBitfield(23, 16, reg) == limit.maxRatioLimit3C &&
                getBitfield(31, 24, reg) == limit.maxRatioLimit4C &&
                getBitfield(39, 32, reg) == limit.maxRatioLimit5C &&
                getBitfield(47, 40, reg) == limit.maxRatioLimit6C &&
                getBitfield(55, 48, reg) == limit.maxRatioLimit7C &&
                getBitfield(63, 56, reg) == limit.maxRatioLimit8C;
        }
    };
}
