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
#include "MCHBAR/MCHBAR.h"

namespace PWTD::Intel {
    class IntelCPU final: public CPUDevice {
    private:
        struct RegistersCache final {
            std::optional<MSR_RAPL_POWER_UNIT::Units> raplPowerUnit;
            PWTS::ROData<int> temperatureTarget;
        };

        mutable PWTS::Intel::FIVRControlUV fivr {0, 0, 0, 0, 0};
        RegistersCache regsCache {};
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
        std::unique_ptr<MSR_UNK_FIVR_CONTROL> msrUnkFivrControl;
        std::unique_ptr<MSR_PKG_CST_CONFIG_CONTROL> msrPkgCstConfigControl;
        std::unique_ptr<MSR_TEMPERATURE_TARGET> msrTemperatureTarget;

        void setupCPUModelRegisters();
        void buildRegistersCache();
        [[nodiscard]] bool hasIA32PkgThermStatusBit() const;
        [[nodiscard]] bool hasTurboBoostTechBit() const;
        [[nodiscard]] bool hasEnhancedSpeedStepBit() const;
        [[nodiscard]] bool hasEnergyPerfBiasBit() const;
        [[nodiscard]] bool hasHWPBit() const;
        [[nodiscard]] bool hasHWPRequestPkgBit() const;
        [[nodiscard]] bool hasHWPReqEPPBit() const;
        [[nodiscard]] bool hasHWPReqActivityWindowBit() const;
        [[nodiscard]] bool hasHWPReqValidBitsBit() const;
        [[nodiscard]] bool hasHWPCtlBit() const;
        void fillPackageData(const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const;
        void fillCoreData(int cpu, const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const;
        void fillThreadData(int cpu, const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const;
        void applyPackageSettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;
        void applyCoreSettings(int cpu, int coreIdx, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;
        void applyThreadSettings(int cpu, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;

    public:
        IntelCPU(const QSharedPointer<cpu_id_t> &cpuID, const QSharedPointer<cpu_raw_data_t> &cpuRawData);

        [[nodiscard]] QSet<PWTS::Feature> getFeatures() const override;
        void fillDaemonPacket(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, PWTS::DaemonPacket &packet) const override;
        [[nodiscard]] QSet<PWTS::DError> applySettings(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const override;
        [[nodiscard]] PWTS::ROData<int> getTemperature() const override;
    };
}
