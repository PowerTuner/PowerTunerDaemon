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
#include "pwtShared/Include/CPU/Intel/TurboPowerCurrentLimit.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class MSR_TURBO_POWER_CURRENT_LIMIT final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x1ac;

    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::TurboPowerCurrentLimit> get() const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return PWTS::RWData<PWTS::Intel::TurboPowerCurrentLimit>({
                .tdpLimit = static_cast<int>(getBitfield(14, 0, reg) * 0.125 * 1000),
                .tdpLimitOverride = getBitfield(15, 15, reg) == 1,
                .tdcLimit = static_cast<int>(getBitfield(30, 16, reg) * 0.125 * 1000),
                .tdcLimitOverride = getBitfield(31, 31, reg) == 1
            }, true);
        }

        [[nodiscard]]
        bool set(const PWTS::RWData<PWTS::Intel::TurboPowerCurrentLimit> &data) const {
            if (!data.isValid())
                return true;

            const PWTS::Intel::TurboPowerCurrentLimit limit = data.getValue();
            const uint64_t tdpLimit = static_cast<uint64_t>(limit.tdpLimit / 0.125 / 1000);
            const uint64_t tdcLimit = static_cast<uint64_t>(limit.tdcLimit / 0.125 / 1000);
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(14, 0, tdpLimit, reg);
            reg = setBitfield(15, 15, limit.tdpLimitOverride, reg);
            reg = setBitfield(30, 16, tdcLimit, reg);
            reg = setBitfield(31, 31, limit.tdcLimitOverride, reg);

            if (!msrDev->write(reg, addr, 0) || !msrDev->read(addr, 0, reg))
                return false;

            return getBitfield(14, 0, reg) == tdpLimit &&
                getBitfield(15, 15, reg) == limit.tdpLimitOverride &&
                getBitfield(30, 16, reg) == tdcLimit &&
                getBitfield(31, 31, reg) == limit.tdcLimitOverride;
        }
    };
}
