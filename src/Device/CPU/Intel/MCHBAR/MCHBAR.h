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
#include "../Include/RegistersIncludes.h"
#include "../../../../external/libPowerTuner/src/DeviceData.h"

namespace PWTD::Intel {
    class MCHBAR final {
    private:
        uint32_t baseAddress;
        MCHBAR_PACKAGE_POWER_SKU_UNIT::Units powerSkuUnit;
        std::unique_ptr<MCHBAR_PACKAGE_RAPL_LIMIT> mchbarPackageRaplLimit;
        bool initialized = false;

        void setupCPUModelRegisters(int family, int extModel);
        void setFeatures(LIBPWT::DeviceData &devData);

    public:
        MCHBAR(int cpuFamily, int cpuExtModel, LIBPWT::DeviceData &devData);

        [[nodiscard]] bool isAccessible() const { return initialized; }

        void read(LIBPWT::DeviceData &devData) const;
        [[nodiscard]] std::vector<LIBPWT::Error> write(const LIBPWT::DeviceData &clientData) const;
    };
}
