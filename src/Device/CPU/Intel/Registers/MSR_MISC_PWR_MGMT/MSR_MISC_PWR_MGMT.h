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

#include "../../../CPURegister.h"
#include "pwtShared/Include/CPU/Intel/MiscPwrMgmt.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class MSR_MISC_PWR_MGMT: public CPURegister {
    public:
        MSR_MISC_PWR_MGMT() {
            addr = 0x1aa;
        }

        virtual PWTS::RWData<PWTS::Intel::MiscPwrMgmt> getMiscPwrMgmtData() const = 0;
        [[nodiscard]] virtual bool setMiscPwrMgmt(const PWTS::RWData<PWTS::Intel::MiscPwrMgmt> &data) const = 0;
    };
}
