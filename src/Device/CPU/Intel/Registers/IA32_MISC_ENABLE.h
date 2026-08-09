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
#include "pwtShared/Include/CPU/Intel/MiscProcFeatures.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class IA32_MISC_ENABLE final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x1a0;

    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::MiscProcFeatures> get() const {
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, 0))
                return {};

            return PWTS::RWData<PWTS::Intel::MiscProcFeatures>({
                .enhancedSpeedStep = getBitfield(16, 16, reg) == 1,
                .disableTurboMode = getBitfield(38, 38, reg) == 1
            }, true);
        }

        [[nodiscard]]
        bool set(const PWTS::RWData<PWTS::Intel::MiscProcFeatures> &data) const {
            if (!data.isValid())
                return true;

            const PWTS::Intel::MiscProcFeatures miscFeat = data.getValue();
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, 0))
                return false;

            reg = setBitfield(16, 16, miscFeat.enhancedSpeedStep, reg);
            reg = setBitfield(38, 38, miscFeat.disableTurboMode, reg);

            if (!msrUtils->writeMSR(reg, addr, 0) || !msrUtils->readMSR(reg, addr, 0))
                return false;

            return getBitfield(16, 16, reg) == miscFeat.enhancedSpeedStep &&
                getBitfield(38, 38, reg) == miscFeat.disableTurboMode;
        }
    };
}
