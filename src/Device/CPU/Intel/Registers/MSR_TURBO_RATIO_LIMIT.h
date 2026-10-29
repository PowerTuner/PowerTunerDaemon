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
#include "../../../../external/libPowerTuner/src/Vendor/Intel/Include/TurboRatioLimit.h"

namespace PWTD::Intel {
    class MSR_TURBO_RATIO_LIMIT final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x1ad;

    public:
        [[nodiscard]]
        std::optional<LIBPWT::Intel::TurboRatioLimit> get() const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return LIBPWT::Intel::TurboRatioLimit {
                .maxRatioLimit1C = static_cast<int>(getBitfield(7, 0, reg)),
                .maxRatioLimit2C = static_cast<int>(getBitfield(15, 8, reg)),
                .maxRatioLimit3C = static_cast<int>(getBitfield(23, 16, reg)),
                .maxRatioLimit4C = static_cast<int>(getBitfield(31, 24, reg)),
                .maxRatioLimit5C = static_cast<int>(getBitfield(39, 32, reg)),
                .maxRatioLimit6C = static_cast<int>(getBitfield(47, 40, reg)),
                .maxRatioLimit7C = static_cast<int>(getBitfield(55, 48, reg)),
                .maxRatioLimit8C = static_cast<int>(getBitfield(63, 56, reg))
            };
        }

        [[nodiscard]]
        bool set(const LIBPWT::Intel::TurboRatioLimit &data) const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(7, 0, data.maxRatioLimit1C, reg);
            reg = setBitfield(15, 8, data.maxRatioLimit2C, reg);
            reg = setBitfield(23, 16, data.maxRatioLimit3C, reg);
            reg = setBitfield(31, 24, data.maxRatioLimit4C, reg);
            reg = setBitfield(39, 32, data.maxRatioLimit5C, reg);
            reg = setBitfield(47, 40, data.maxRatioLimit6C, reg);
            reg = setBitfield(55, 48, data.maxRatioLimit7C, reg);
            reg = setBitfield(63, 56, data.maxRatioLimit8C, reg);

            return msrDev->write(reg, addr, 0) &&
                msrDev->read(addr, 0, reg) &&
                getBitfield(7, 0, reg) == data.maxRatioLimit1C &&
                getBitfield(15, 8, reg) == data.maxRatioLimit2C &&
                getBitfield(23, 16, reg) == data.maxRatioLimit3C &&
                getBitfield(31, 24, reg) == data.maxRatioLimit4C &&
                getBitfield(39, 32, reg) == data.maxRatioLimit5C &&
                getBitfield(47, 40, reg) == data.maxRatioLimit6C &&
                getBitfield(55, 48, reg) == data.maxRatioLimit7C &&
                getBitfield(63, 56, reg) == data.maxRatioLimit8C;
        }
    };
}
