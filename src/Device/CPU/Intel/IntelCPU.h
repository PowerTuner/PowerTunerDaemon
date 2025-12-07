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
            PWTS::ROData<MSR_RAPL_POWER_UNIT::RAPLPowerUnits> raplPowerUnit;
            PWTS::ROData<int> temperatureTarget;
        };

        mutable PWTS::Intel::FIVRControlUV fivr {0, 0, 0, 0, 0};
        mutable QScopedPointer<RegistersCache> regsCache;
        QScopedPointer<MCHBAR> mchbar;
        QScopedPointer<IA32_ENERGY_PERF_BIAS> ia32EnergyPerfBias;
        QScopedPointer<IA32_MISC_ENABLE> ia32MiscEnable;
        QScopedPointer<IA32_PM_ENABLE> ia32PmEnable;
        QScopedPointer<IA32_PACKAGE_THERM_STATUS> ia32PackageThermStatus;
        QScopedPointer<IA32_HWP_CAPABILITIES> ia32HWPCapabilities;
        QScopedPointer<IA32_HWP_REQUEST_PKG> ia32HWPRequestPkg;
        QScopedPointer<IA32_HWP_REQUEST> ia32HWPRequest;
        QScopedPointer<IA32_HWP_CTL> ia32HwpCtl;
        QScopedPointer<MSR_PLATFORM_INFO> msrPlatformInfo;
        QScopedPointer<MSR_PKG_POWER_LIMIT> msrPkgPowerLimit;
        QScopedPointer<MSR_VR_CURRENT_CONFIG> msrVrCurrentConfig;
        QScopedPointer<MSR_PP1_CURRENT_CONFIG> msrPP1CurrentConfig;
        QScopedPointer<MSR_TURBO_POWER_CURRENT_LIMIT> msrTurboPowerCurrentLimit;
        QScopedPointer<MSR_PP0_POLICY> msrPP0Policy;
        QScopedPointer<MSR_PP1_POLICY> msrPP1Policy;
        QScopedPointer<MSR_TURBO_RATIO_LIMIT> msrTurboRatioLimit;
        QScopedPointer<MSR_POWER_CTL> msrPowerCtl;
        QScopedPointer<MSR_MISC_PWR_MGMT> msrMiscPwrMgmt;
        QScopedPointer<MSR_UNK_FIVR_CONTROL> msrUnkFivrControl;
        QScopedPointer<MSR_PKG_CST_CONFIG_CONTROL> msrPkgCstConfigControl;
        QScopedPointer<MSR_TEMPERATURE_TARGET> msrTemperatureTarget;

        void setupCPUModelRegisters();
        void cacheStaticRegistersData() const;
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
