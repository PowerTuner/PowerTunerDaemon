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
#include "../CPUDevice.h"
#include "../../external/libcpuid/libcpuid/libcpuid.h"
#include "Includes/RegistersInlcudes.h"
#include "SMU/RyzenAdj.h"

namespace PWTD::AMD {
    class AMDCPU final: public CPUDevice {
    private:
        std::unique_ptr<RyzenAdj> ryzenAdj;
        std::unique_ptr<MSR_PSTATE_CURRENT_LIMIT> msrPStateCurrentLimit;
        std::unique_ptr<MSR_PSTATE_CONTROL> msrPStateControl;
        std::unique_ptr<MSR_CORE_PERFORMANCE_BOOST> msrCorePerformanceBoost;
        std::unique_ptr<MSR_CPPC_CAPABILITY_1> msrCppcCapability1;
        std::unique_ptr<MSR_CPPC_ENABLE> msrCppcEnable;
        std::unique_ptr<MSR_CPPC_REQUEST> msrCppcRequest;

        [[nodiscard]] bool hasCorePerformanceBoostBit(const cpu_raw_data_t &cpuidRaw) const;
        [[nodiscard]] bool hasHWPStateBit(const cpu_raw_data_t &cpuidRaw) const;
        [[nodiscard]] bool hasCPPCBit(const cpu_raw_data_t &cpuidRaw) const;
        void readThread(int cpu, LIBPWT::DeviceData &devData) const;
        void writeThread(int cpu, const LIBPWT::DeviceData &clientData, std::vector<LIBPWT::Error> &errors) const;

    public:
        AMDCPU(const cpu_raw_data_t &cpuidRaw, LIBPWT::DeviceData &devData);

        void read(LIBPWT::DeviceData &devData) const override;
        [[nodiscard]] std::vector<LIBPWT::Error> write(LIBPWT::DeviceData &devData, const LIBPWT::DeviceData &clientData) const override;
        [[nodiscard]] std::optional<int> getTemperature() const override;
    };
}
