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
#include "MSR_VR_CURRENT_CONFIG.h"
#include "../../../Utils/Utils.h"

namespace PWTD::Intel {
    class MSR_VR_CURRENT_CONFIG_CU1 final: public MSR_VR_CURRENT_CONFIG {
    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::VRCurrentConfig> get() const override {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return PWTS::RWData<PWTS::Intel::VRCurrentConfig>({
                .pl4 = static_cast<int>(static_cast<double>(getBitfield(15, 0, reg)) * 0.125 * 1000),
                .lock = getBitfield(31, 31, reg) == 1
            }, true);
        }

        [[nodiscard]]
        bool set(const PWTS::RWData<PWTS::Intel::VRCurrentConfig> &data) const override {
            if (!data.isValid())
                return true;

            const PWTS::Intel::VRCurrentConfig vrCfg = data.getValue();
            const int pl4 = static_cast<int>(vrCfg.pl4 / 0.125 / 1000);
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(15, 0, pl4, reg);
            reg = setBitfield(31, 31, vrCfg.lock, reg);

            if (!msrDev->write(reg, addr, 0) || !msrDev->read(addr, 0, reg))
                return false;

            return getBitfield(15, 0, reg) == pl4 &&
                getBitfield(31, 31, reg) == vrCfg.lock;
        }
    };
}
