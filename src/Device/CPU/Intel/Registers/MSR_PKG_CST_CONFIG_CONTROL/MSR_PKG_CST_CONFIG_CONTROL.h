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
#include "pwtShared/Include/CPU/Intel/PkgCstConfigControl.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class MSR_PKG_CST_CONFIG_CONTROL: public CPURegister {
    public:
        MSR_PKG_CST_CONFIG_CONTROL() {
            addr = 0xe2;
        }

        virtual PWTS::RWData<PWTS::Intel::PkgCstConfigControl> getPkgCstConfigControlData(int cpu) const = 0;
        [[nodiscard]] virtual bool setPkgCstConfigControlData(int cpu, const PWTS::RWData<PWTS::Intel::PkgCstConfigControl> &data) const = 0;
    };
}
