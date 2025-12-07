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

#include "../MCHBAR_PACKAGE_POWER_SKU_UNIT.h"
#include "pwtShared/Include/CPU/Intel/MCHBARPkgRaplLimit.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class MCHBAR_PACKAGE_RAPL_LIMIT: public MCHBARRegister {
    public:
        explicit MCHBAR_PACKAGE_RAPL_LIMIT(const uint32_t base): MCHBARRegister(base, 0x59a0) {}
        virtual ~MCHBAR_PACKAGE_RAPL_LIMIT() = default;

        virtual PWTS::RWData<PWTS::Intel::MCHBARPkgRaplLimit> getPkgRaplLimitData(const PWTS::ROData<MCHBAR_PACKAGE_POWER_SKU_UNIT::PkgPowerSKUUnits> &powerSkuUnit) const = 0;
        [[nodiscard]] virtual bool setPkgRaplLimit(const PWTS::RWData<PWTS::Intel::MCHBARPkgRaplLimit> &data, const PWTS::ROData<MCHBAR_PACKAGE_POWER_SKU_UNIT::PkgPowerSKUUnits> &powerSkuUnit) const = 0;
    };
}
