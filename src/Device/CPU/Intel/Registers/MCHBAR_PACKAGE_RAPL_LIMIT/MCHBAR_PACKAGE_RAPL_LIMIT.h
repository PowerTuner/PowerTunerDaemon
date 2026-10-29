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
#include "../../../../../external/libPowerTuner/src/Vendor/Intel/Include/MCHBARPkgRaplLimit.h"

namespace PWTD::Intel {
    class MCHBAR_PACKAGE_RAPL_LIMIT: public MCHBARRegister {
    public:
        explicit MCHBAR_PACKAGE_RAPL_LIMIT(const uint32_t base): MCHBARRegister(base, 0x59a0) {}
        virtual ~MCHBAR_PACKAGE_RAPL_LIMIT() = default;

        [[nodiscard]] virtual std::optional<LIBPWT::Intel::MCHBARPkgRaplLimit> get(const MCHBAR_PACKAGE_POWER_SKU_UNIT::Units &pu) const = 0;
        [[nodiscard]] virtual bool set(const LIBPWT::Intel::MCHBARPkgRaplLimit &data, const MCHBAR_PACKAGE_POWER_SKU_UNIT::Units &pu) const = 0;
    };
}
