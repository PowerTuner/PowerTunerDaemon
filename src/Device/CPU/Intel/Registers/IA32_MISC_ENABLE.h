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
#include "../../../../external/libPowerTuner/src/Vendor/Intel/Include/MiscProcFeatures.h"

namespace PWTD::Intel {
    class IA32_MISC_ENABLE final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x1a0;

    public:
        [[nodiscard]]
        std::optional<LIBPWT::Intel::MiscProcFeatures> get() const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return LIBPWT::Intel::MiscProcFeatures {
                .enhancedSpeedStep = getBitfield(16, 16, reg) == 1,
                .disableTurboMode = getBitfield(38, 38, reg) == 1
            };
        }

        [[nodiscard]]
        bool set(const LIBPWT::Intel::MiscProcFeatures &data) const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(16, 16, data.enhancedSpeedStep, reg);
            reg = setBitfield(38, 38, data.disableTurboMode, reg);

            return msrDev->write(reg, addr, 0) &&
                msrDev->read(addr, 0, reg) &&
                getBitfield(16, 16, reg) == data.enhancedSpeedStep &&
                getBitfield(38, 38, reg) == data.disableTurboMode;
        }
    };
}
