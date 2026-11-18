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
#include "MCHBAR/MCHBAR.h"

namespace PWTD::Intel {
    class IntelCPU final: public CPUDevice {
    private:
        MSR_RAPL_POWER_UNIT::Units powerUnits;
        int baseTCCTemperatureTarget;
        std::unique_ptr<MCHBAR> mchbar;
        std::unique_ptr<IA32_ENERGY_PERF_BIAS> ia32EnergyPerfBias;
        std::unique_ptr<IA32_MISC_ENABLE> ia32MiscEnable;
        std::unique_ptr<IA32_PM_ENABLE> ia32PmEnable;
        std::unique_ptr<IA32_PACKAGE_THERM_STATUS> ia32PackageThermStatus;
        std::unique_ptr<IA32_HWP_CAPABILITIES> ia32HWPCapabilities;
        std::unique_ptr<IA32_HWP_REQUEST_PKG> ia32HWPRequestPkg;
        std::unique_ptr<IA32_HWP_REQUEST> ia32HWPRequest;
        std::unique_ptr<IA32_HWP_CTL> ia32HwpCtl;
        std::unique_ptr<MSR_PLATFORM_INFO> msrPlatformInfo;
        std::unique_ptr<MSR_PKG_POWER_LIMIT> msrPkgPowerLimit;
        std::unique_ptr<MSR_VR_CURRENT_CONFIG> msrVrCurrentConfig;
        std::unique_ptr<MSR_PP1_CURRENT_CONFIG> msrPP1CurrentConfig;
        std::unique_ptr<MSR_TURBO_POWER_CURRENT_LIMIT> msrTurboPowerCurrentLimit;
        std::unique_ptr<MSR_PP0_POLICY> msrPP0Policy;
        std::unique_ptr<MSR_PP1_POLICY> msrPP1Policy;
        std::unique_ptr<MSR_TURBO_RATIO_LIMIT> msrTurboRatioLimit;
        std::unique_ptr<MSR_POWER_CTL> msrPowerCtl;
        std::unique_ptr<MSR_MISC_PWR_MGMT> msrMiscPwrMgmt;
        std::unique_ptr<MSR_UNK_FIVR_CONTROL> msrFivrControl;
        std::unique_ptr<MSR_PKG_CST_CONFIG_CONTROL> msrPkgCstConfigControl;
        std::unique_ptr<MSR_TEMPERATURE_TARGET> msrTemperatureTarget;

        void setupCPUModelRegisters(int family, int extModel);
        [[nodiscard]] bool hasIA32PkgThermStatusBit(const cpu_raw_data_t &cpuidRaw) const;
        [[nodiscard]] bool hasTurboBoostTechBit(const cpu_raw_data_t &cpuidRaw) const;
        [[nodiscard]] bool hasEnhancedSpeedStepBit(const cpu_raw_data_t &cpuidRaw) const;
        [[nodiscard]] bool hasEnergyPerfBiasBit(const cpu_raw_data_t &cpuidRaw) const;
        [[nodiscard]] bool hasHWPBit(const cpu_raw_data_t &cpuidRaw) const;
        [[nodiscard]] bool hasHWPRequestPkgBit(const cpu_raw_data_t &cpuidRaw) const;
        [[nodiscard]] bool hasHWPReqEPPBit(const cpu_raw_data_t &cpuidRaw) const;
        [[nodiscard]] bool hasHWPReqActivityWindowBit(const cpu_raw_data_t &cpuidRaw) const;
        [[nodiscard]] bool hasHWPReqValidBitsBit(const cpu_raw_data_t &cpuidRaw) const;
        [[nodiscard]] bool hasHWPCtlBit(const cpu_raw_data_t &cpuidRaw) const;
        void setFeatures(const cpu_raw_data_t &cpuidRaw, LIBPWT::DeviceData &devData);
        void readCore(int cpu, LIBPWT::DeviceData &devData) const;
        void readThread(int cpu, LIBPWT::DeviceData &devData) const;
        void writeCore(int cpu, const LIBPWT::DeviceData &clientData, std::vector<LIBPWT::Error> &errors) const;
        void writeThread(int cpu, const LIBPWT::DeviceData &clientData, std::vector<LIBPWT::Error> &errors) const;

    public:
        IntelCPU(const cpu_id_t &cpuid, const cpu_raw_data_t &cpuidRaw, LIBPWT::DeviceData &devData);

        void read(LIBPWT::DeviceData &devData) const override;
        [[nodiscard]] std::vector<LIBPWT::Error> write(LIBPWT::DeviceData &devData, const LIBPWT::DeviceData &clientData) const override;
        [[nodiscard]] std::optional<int> getTemperature() const override;
    };
}
