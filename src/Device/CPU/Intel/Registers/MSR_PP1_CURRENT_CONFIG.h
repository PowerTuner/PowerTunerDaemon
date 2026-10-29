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
#include "../../../../external/libPowerTuner/src/Vendor/Intel/Include/PP1CurrentConfig.h"

namespace PWTD::Intel {
    class MSR_PP1_CURRENT_CONFIG final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x602;

    public:
        [[nodiscard]]
        std::optional<LIBPWT::Intel::PP1CurrentConfig> get() const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return LIBPWT::Intel::PP1CurrentConfig {
                .limit = static_cast<int>(getBitfield(12, 0, reg) * 0.125 * 1000),
                .lock = getBitfield(31, 31, reg) == 1
            };
        }

        [[nodiscard]]
        bool set(const LIBPWT::Intel::PP1CurrentConfig &data) const {
            const uint64_t limit = static_cast<uint64_t>(data.limit / 0.125 / 1000);
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(12, 0, limit, reg);
            reg = setBitfield(31, 31, data.lock, reg);

            return msrDev->write(reg, addr, 0) &&
                msrDev->read(addr, 0, reg) &&
                getBitfield(12, 0, reg) == limit &&
                getBitfield(31, 31, reg) == data.lock;
        }
    };
}
