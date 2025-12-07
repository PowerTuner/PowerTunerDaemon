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
#include "IntelCPU.h"
#include "Include/ModelRegistersIncludes.h"
#include "Include/CPUFamily.h"
#include "Include/CPUModel.h"

namespace PWTD::Intel {
    IntelCPU::IntelCPU(const QSharedPointer<cpu_id_t> &cpuID, const QSharedPointer<cpu_raw_data_t> &cpuRawData): CPUDevice(cpuID, cpuRawData) {
        cpuInfo->vendor = PWTS::CPUVendor::Intel;
        msrDev = MSRFactory::getMSRInstance();

        mchbar.reset(new MCHBAR(cpuID->x86.family));
        if (!mchbar->init(cpuID->x86.family, cpuID->x86.ext_model))
            mchbar.reset();

        setupCPUModelRegisters();
        ia32MiscEnable.reset(new IA32_MISC_ENABLE);
        msrTurboRatioLimit.reset(new MSR_TURBO_RATIO_LIMIT);

        if (hasIA32PkgThermStatusBit())
            ia32PackageThermStatus.reset(new IA32_PACKAGE_THERM_STATUS);

        if (hasEnergyPerfBiasBit())
            ia32EnergyPerfBias.reset(new IA32_ENERGY_PERF_BIAS);

        if (hasHWPBit()) {
            ia32PmEnable.reset(new IA32_PM_ENABLE);
            ia32HWPCapabilities.reset(new IA32_HWP_CAPABILITIES);
            ia32HWPRequest.reset(new IA32_HWP_REQUEST);

            if (hasHWPRequestPkgBit())
                ia32HWPRequestPkg.reset(new IA32_HWP_REQUEST_PKG);

            if (hasHWPCtlBit())
                ia32HwpCtl.reset(new IA32_HWP_CTL);
        }

        cacheStaticRegistersData();
    }

    void IntelCPU::setupCPUModelRegisters() {
        switch (cpuInfo->family) {
            case Family6: {
                switch (cpuInfo->extModel) {
                    case Clarkdale: {
                        msrMiscPwrMgmt.reset(new MSR_MISC_PWR_MGMT_NHLM);
                        msrPlatformInfo.reset(new MSR_PLATFORM_INFO_NHLM);
                        msrTurboPowerCurrentLimit.reset(new MSR_TURBO_POWER_CURRENT_LIMIT);
                        msrPowerCtl.reset(new MSR_POWER_CTL_NHLM);
                        msrTemperatureTarget.reset(new MSR_TEMPERATURE_TARGET_NHLM);
                    }
                        break;
                    case SandyBridge: {
                        msrMiscPwrMgmt.reset(new MSR_MISC_PWR_MGMT_NHLM);
                        msrPlatformInfo.reset(new MSR_PLATFORM_INFO_NHLM);
                        msrPkgPowerLimit.reset(new MSR_PKG_POWER_LIMIT);
                        msrVrCurrentConfig.reset(new MSR_VR_CURRENT_CONFIG_SB);
                        msrPP0Policy.reset(new MSR_PP0_POLICY);
                        msrPP1Policy.reset(new MSR_PP1_POLICY);
                        msrPP1CurrentConfig.reset(new MSR_PP1_CURRENT_CONFIG);
                        msrPkgCstConfigControl.reset(new MSR_PKG_CST_CONFIG_CONTROL_SB);
                        msrPowerCtl.reset(new MSR_POWER_CTL_SB);
                        msrTemperatureTarget.reset(new MSR_TEMPERATURE_TARGET_NHLM);
                    }
                        break;
                    case IvyBridge: {
                        msrMiscPwrMgmt.reset(new MSR_MISC_PWR_MGMT_NHLM);
                        msrPlatformInfo.reset(new MSR_PLATFORM_INFO_IVB);
                        msrPkgPowerLimit.reset(new MSR_PKG_POWER_LIMIT);
                        msrVrCurrentConfig.reset(new MSR_VR_CURRENT_CONFIG_SB);
                        msrPP0Policy.reset(new MSR_PP0_POLICY);
                        msrPP1Policy.reset(new MSR_PP1_POLICY);
                        msrPP1CurrentConfig.reset(new MSR_PP1_CURRENT_CONFIG);
                        msrPkgCstConfigControl.reset(new MSR_PKG_CST_CONFIG_CONTROL_SB);
                        msrPowerCtl.reset(new MSR_POWER_CTL_SB);
                        msrTemperatureTarget.reset(new MSR_TEMPERATURE_TARGET_NHLM);
                    }
                        break;
                    case IceLakeU: {
                        msrMiscPwrMgmt.reset(new MSR_MISC_PWR_MGMT_NHLM);
                        msrPlatformInfo.reset(new MSR_PLATFORM_INFO_IVB);
                        msrPkgPowerLimit.reset(new MSR_PKG_POWER_LIMIT);
                        msrVrCurrentConfig.reset(new MSR_VR_CURRENT_CONFIG_SB);
                        msrPP0Policy.reset(new MSR_PP0_POLICY);
                        msrPP1Policy.reset(new MSR_PP1_POLICY);
                        msrPowerCtl.reset(new MSR_POWER_CTL_SB);
                        msrUnkFivrControl.reset(new MSR_UNK_FIVR_CONTROL_ICL);
                        msrTemperatureTarget.reset(new MSR_TEMPERATURE_TARGET_NHLM);
                    }
                        break;
                    case TigerLakeU: {
                        msrMiscPwrMgmt.reset(new MSR_MISC_PWR_MGMT_NHLM);
                        msrPlatformInfo.reset(new MSR_PLATFORM_INFO_IVB);
                        msrPkgPowerLimit.reset(new MSR_PKG_POWER_LIMIT);
                        msrVrCurrentConfig.reset(new MSR_VR_CURRENT_CONFIG_SB);
                        msrPP0Policy.reset(new MSR_PP0_POLICY);
                        msrPP1Policy.reset(new MSR_PP1_POLICY);
                        msrPowerCtl.reset(new MSR_POWER_CTL_SB);
                        msrTemperatureTarget.reset(new MSR_TEMPERATURE_TARGET_NHLM);
                    }
                        break;
                    case AlderLakeN: {
                        msrPlatformInfo.reset(new MSR_PLATFORM_INFO_IVB);
                        msrPkgPowerLimit.reset(new MSR_PKG_POWER_LIMIT);
                        msrVrCurrentConfig.reset(new MSR_VR_CURRENT_CONFIG_SB);
                        msrPP0Policy.reset(new MSR_PP0_POLICY);
                        msrPP1Policy.reset(new MSR_PP1_POLICY);
                        msrPowerCtl.reset(new MSR_POWER_CTL_SB);
                        msrTemperatureTarget.reset(new MSR_TEMPERATURE_TARGET_NHLM);
                    }
                        break;
                    case LunarLake: {
                        msrPlatformInfo.reset(new MSR_PLATFORM_INFO_IVB);
                        msrPkgPowerLimit.reset(new MSR_PKG_POWER_LIMIT);
                        msrVrCurrentConfig.reset(new MSR_VR_CURRENT_CONFIG_CU1);
                        msrPP0Policy.reset(new MSR_PP0_POLICY);
                        msrPP1Policy.reset(new MSR_PP1_POLICY);
                        msrPowerCtl.reset(new MSR_POWER_CTL_CU1);
                        msrPkgCstConfigControl.reset(new MSR_PKG_CST_CONFIG_CONTROL_CU1);
                        msrTemperatureTarget.reset(new MSR_TEMPERATURE_TARGET_NHLM);
                    }
                        break;
                    default:
                        break;
                }
            }
                break;
            default:
                break;
        }
    }

    void IntelCPU::cacheStaticRegistersData() const {
        regsCache.reset(new RegistersCache);

        if (msrDev->openMsrFd(0)) {
            if (!msrPkgPowerLimit.isNull()) {
                const std::unique_ptr<MSR_RAPL_POWER_UNIT> msrRaplPowUnit = std::make_unique<MSR_RAPL_POWER_UNIT>();

                regsCache->raplPowerUnit = msrRaplPowUnit->getPowerUnitData();
            }

            if (!msrTemperatureTarget.isNull()) {
                const PWTS::RWData<PWTS::Intel::TemperatureTarget> tempTarget = msrTemperatureTarget->getTemperatureTargetData();

                if (tempTarget.isValid())
                    regsCache->temperatureTarget = PWTS::ROData<int>(tempTarget.getValue().temperatureTarget, true);
            }

            msrDev->closeMsrFd(0);

        } else if (logger->isLevel(PWTS::LogLevel::Error)) {
            logger->write("failed to create registers cache");
        }
    }

    bool IntelCPU::hasIA32PkgThermStatusBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];
        bool ret;

        try {
            ret = getBitfield(6, 6, eax) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    bool IntelCPU::hasEnergyPerfBiasBit() const {
        const uint32_t ecx = cpuidRaw->basic_cpuid[6][cpu_registers_t::ECX];
        bool ret;

        try {
            ret = getBitfield(3, 3, ecx) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    bool IntelCPU::hasTurboBoostTechBit() const {
        const PWTS::RWData<PWTS::Intel::MiscProcFeatures> miscFeaturesData = ia32MiscEnable->getMiscProcessorFeaturesData();
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];
        const int disableTurbo = !miscFeaturesData.isValid() ? 0 : miscFeaturesData.getValue().disableTurboMode;
        bool ret;

        try { // eax bit is cleared when disabled, so show the feature even when disable is 1
            ret = getBitfield(1, 1, eax) == 1 || disableTurbo == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    bool IntelCPU::hasEnhancedSpeedStepBit() const {
        const uint32_t ecx = cpuidRaw->basic_cpuid[1][cpu_registers_t::ECX];
        bool ret;

        try {
            ret = getBitfield(7, 7, ecx) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    bool IntelCPU::hasHWPBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];
        bool ret;

        try {
            ret = getBitfield(7, 7, eax) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    bool IntelCPU::hasHWPReqActivityWindowBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];
        bool ret;

        try {
            ret = getBitfield(9, 9, eax) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    bool IntelCPU::hasHWPReqEPPBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];
        bool ret;

        try {
            ret = getBitfield(10, 10, eax) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    bool IntelCPU::hasHWPRequestPkgBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];
        bool ret;

        try {
            ret = getBitfield(11, 11, eax) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    bool IntelCPU::hasHWPReqValidBitsBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];
        bool ret;

        try {
            ret = getBitfield(17, 17, eax) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    bool IntelCPU::hasHWPCtlBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];
        bool ret;

        try {
            ret = getBitfield(22, 22, eax) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    QSet<PWTS::Feature> IntelCPU::getFeatures() const {
        if (!msrDev->openMsrFd(0)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("failed to open msr fd"));

            return {};
        }

        QSet<PWTS::Feature> features;
        bool canProgTDPLimitForTurboMode = false;
        bool canProgRatioLimitForTurboMode = false;

        if (!msrPlatformInfo.isNull()) {
            const PWTS::ROData<PlatformInfoData> data = msrPlatformInfo->getPlatformInfoData();

            features.unite({PWTS::Feature::INTEL_PLATFORM_INFO, PWTS::Feature::INTEL_CPU_GROUP});

            if (dynamic_cast<MSR_PLATFORM_INFO_NHLM *>(msrPlatformInfo.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_PLATFORM_INFO_NHLM);
            else if (dynamic_cast<MSR_PLATFORM_INFO_IVB *>(msrPlatformInfo.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_PLATFORM_INFO_IVB);

            if (data.isValid()) {
                const PlatformInfoData val = data.getValue();

                canProgTDPLimitForTurboMode = val.programmableTDPLimitForTurboMode;
                canProgRatioLimitForTurboMode = val.programmableRatioLimitForTurboMode;
            }
        }

        if (!ia32PackageThermStatus.isNull())
            features.unite({PWTS::Feature::INTEL_PKG_THERM_STATUS, PWTS::Feature::INTEL_CPU_STAT_GROUP});

        if (!msrPkgPowerLimit.isNull())
            features.unite({PWTS::Feature::INTEL_PKG_POWER_LIMIT, PWTS::Feature::INTEL_CPU_GROUP});

        if (!msrVrCurrentConfig.isNull())
            features.unite({PWTS::Feature::INTEL_VR_CURRENT_CFG, PWTS::Feature::INTEL_CPU_GROUP});

        if (!msrPP1CurrentConfig.isNull())
            features.unite({PWTS::Feature::INTEL_PP1_CURRENT_CFG, PWTS::Feature::INTEL_CPU_GROUP});

        if (!msrTurboPowerCurrentLimit.isNull()) {
            features.unite({PWTS::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT, PWTS::Feature::INTEL_CPU_GROUP});

            if (canProgTDPLimitForTurboMode)
                features.insert(PWTS::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT_RW);
        }

        if (!msrPP0Policy.isNull())
            features.unite({PWTS::Feature::INTEL_CPU_POWER_BALANCE, PWTS::Feature::INTEL_CPU_GROUP});

        if (!msrPP1Policy.isNull())
            features.unite({PWTS::Feature::INTEL_GPU_POWER_BALANCE, PWTS::Feature::INTEL_CPU_GROUP});

        if (!ia32EnergyPerfBias.isNull())
            features.unite({PWTS::Feature::INTEL_ENERGY_PERF_BIAS, PWTS::Feature::INTEL_CPU_GROUP});

        if (!msrTurboRatioLimit.isNull()) {
            features.unite({PWTS::Feature::INTEL_TURBO_RATIO_LIMIT, PWTS::Feature::INTEL_CPU_GROUP});

            if (canProgRatioLimitForTurboMode)
                features.insert(PWTS::Feature::INTEL_TURBO_RATIO_LIMIT_RW);
        }

        if (hasTurboBoostTechBit())
            features.unite({PWTS::Feature::INTEL_TURBO_BOOST, PWTS::Feature::INTEL_CPU_GROUP, PWTS::Feature::INTEL_IA32_MISC_ENABLE_GROUP});

        if (hasEnhancedSpeedStepBit())
            features.unite({PWTS::Feature::INTEL_ENHANCED_SPEEDSTEP, PWTS::Feature::INTEL_CPU_GROUP, PWTS::Feature::INTEL_IA32_MISC_ENABLE_GROUP});

        if (!msrPowerCtl.isNull()) {
            features.unite({PWTS::Feature::INTEL_POWER_CTL, PWTS::Feature::INTEL_CPU_GROUP});

            if (dynamic_cast<MSR_POWER_CTL_NHLM *>(msrPowerCtl.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_POWER_CTL_NHLM);
            else if (dynamic_cast<MSR_POWER_CTL_SB *>(msrPowerCtl.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_POWER_CTL_SB);
            else if (dynamic_cast<MSR_POWER_CTL_CU1 *>(msrPowerCtl.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_POWER_CTL_CU1);
        }

        if (!msrMiscPwrMgmt.isNull()) {
            features.unite({PWTS::Feature::INTEL_MISC_PWR_MGMT, PWTS::Feature::INTEL_CPU_GROUP});

            if (dynamic_cast<MSR_MISC_PWR_MGMT_NHLM *>(msrMiscPwrMgmt.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_MISC_PWR_MGMT_NHLM);
        }

        if (!msrUnkFivrControl.isNull()) {
            const PWTS::ROData<MSR_UNK_FIVR_CONTROL::FIVRCapabilities> fivrCaps = msrUnkFivrControl->getFIVRCapabilities();

            if (fivrCaps.isValid()) {
                const MSR_UNK_FIVR_CONTROL::FIVRCapabilities caps = fivrCaps.getValue();

                features.unite({PWTS::Feature::INTEL_UNDERVOLT_GROUP, PWTS::Feature::INTEL_CPU_GROUP});

                if (caps.cpu)
                    features.insert(PWTS::Feature::INTEL_UNDERVOLT_CPU);

                if (caps.gpu)
                    features.insert(PWTS::Feature::INTEL_UNDERVOLT_GPU);

                if (caps.cpuCache)
                    features.insert(PWTS::Feature::INTEL_UNDERVOLT_CACHE);

                if (caps.unslice)
                    features.insert(PWTS::Feature::INTEL_UNDERVOLT_UNSLICE);

                if (caps.sysAgent)
                    features.insert(PWTS::Feature::INTEL_UNDERVOLT_SYSAGENT);
            }
        }

        if (!ia32PmEnable.isNull()) {
            features.insert(PWTS::Feature::INTEL_HWP_GROUP);

            if (!ia32HWPRequestPkg.isNull())
                features.insert(PWTS::Feature::INTEL_HWP_REQ_PKG);

            if (hasHWPReqEPPBit())
                features.insert(PWTS::Feature::INTEL_HWP_EPP);

            if (hasHWPReqActivityWindowBit())
                features.insert(PWTS::Feature::INTEL_HWP_ACT_WIND);

            if (hasHWPReqValidBitsBit())
                features.insert(PWTS::Feature::INTEL_HWP_VALID_BITS);

            if (!ia32HwpCtl.isNull())
                features.insert(PWTS::Feature::INTEL_HWP_CTL);
        }

        if (!msrPkgCstConfigControl.isNull()) {
            features.unite({PWTS::Feature::INTEL_PKG_CST_CONFIG_CONTROL, PWTS::Feature::INTEL_CPU_GROUP});

            if (dynamic_cast<MSR_PKG_CST_CONFIG_CONTROL_SB *>(msrPkgCstConfigControl.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_PKG_CST_CONFIG_CONTROL_SB);
            else if (dynamic_cast<MSR_PKG_CST_CONFIG_CONTROL_CU1 *>(msrPkgCstConfigControl.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_PKG_CST_CONFIG_CONTROL_CU1);
        }

        if (!msrTemperatureTarget.isNull())
            features.unite({PWTS::Feature::INTEL_TEMPERATURE_TARGET, PWTS::Feature::INTEL_CPU_GROUP});

        msrDev->closeMsrFd(0);

        if (!mchbar.isNull())
            features.unite(mchbar->getFeatures());

        return features;
    }

    void IntelCPU::fillPackageData(const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::INTEL_CPU_GROUP))
            return;

        if (!msrDev->openMsrFd(0)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("failed to open msr fd"));

            packet.errors.insert(PWTS::DError::NO_MSR_FD);
            return;
        }

        if (features.contains(PWTS::Feature::INTEL_PKG_POWER_LIMIT))
            packet.intelData->pkgPowerLimit = msrPkgPowerLimit->getPkgPowerLimitData(regsCache->raplPowerUnit);

        if (features.contains(PWTS::Feature::INTEL_VR_CURRENT_CFG))
            packet.intelData->vrCurrentCfg = msrVrCurrentConfig->getVrCurrentConfigData();

        if (features.contains(PWTS::Feature::INTEL_PP1_CURRENT_CFG))
            packet.intelData->pp1CurrentCfg = msrPP1CurrentConfig->getPP1CurrentConfigData();

        if (features.contains(PWTS::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT))
            packet.intelData->turboPowerCurrentLimit = msrTurboPowerCurrentLimit->getTurboPowerCurrentLimitData();

        if (features.contains(PWTS::Feature::INTEL_CPU_POWER_BALANCE))
            packet.intelData->pp0Priority = msrPP0Policy->getPP0Priority();

        if (features.contains(PWTS::Feature::INTEL_GPU_POWER_BALANCE))
            packet.intelData->pp1Priority = msrPP1Policy->getPP1Priority();

        if (features.contains(PWTS::Feature::INTEL_ENERGY_PERF_BIAS))
            packet.intelData->energyPerfBias = ia32EnergyPerfBias->getPowerPolicyPreference();

        if (features.contains(PWTS::Feature::INTEL_TURBO_RATIO_LIMIT))
            packet.intelData->turboRatioLimit = msrTurboRatioLimit->getTurboRatioLimitData();

        if (features.contains(PWTS::Feature::INTEL_IA32_MISC_ENABLE_GROUP))
            packet.intelData->miscProcFeatures = ia32MiscEnable->getMiscProcessorFeaturesData();

        if (features.contains(PWTS::Feature::INTEL_POWER_CTL))
            packet.intelData->powerCtl = msrPowerCtl->getPowerCtlData();

        if (features.contains(PWTS::Feature::INTEL_MISC_PWR_MGMT))
            packet.intelData->miscPwrMgmt = msrMiscPwrMgmt->getMiscPwrMgmtData();

        if (features.contains(PWTS::Feature::INTEL_UNDERVOLT_GROUP))
            packet.intelData->undervoltData = PWTS::RWData<PWTS::Intel::FIVRControlUV>(fivr, true);

        if (features.contains(PWTS::Feature::INTEL_HWP_GROUP)) {
            packet.intelData->hwpEnable = ia32PmEnable->getHWPEnableBit();

            if (features.contains(PWTS::Feature::INTEL_HWP_REQ_PKG))
                packet.intelData->hwpRequestPkg = ia32HWPRequestPkg->getHWPRequestPkgData();

            if (features.contains(PWTS::Feature::INTEL_HWP_CTL))
                packet.intelData->hwpPkgCtlPolarity = ia32HwpCtl->getHwpCtlBit();
        }

        msrDev->closeMsrFd(0);
    }

    void IntelCPU::fillCoreData(const int cpu, const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const {
        PWTS::Intel::IntelCoreData coreData {};

        if (!features.contains(PWTS::Feature::INTEL_CPU_GROUP)) {
            packet.intelData->coreData.append(coreData);
            return;
        }

        if (!msrDev->openMsrFd(cpu)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to open msr fd for cpu %1").arg(cpu));

            packet.errors.insert(PWTS::DError::NO_MSR_FD);
            packet.intelData->coreData.append(coreData);
            return;
        }

        if (features.contains(PWTS::Feature::INTEL_PKG_CST_CONFIG_CONTROL))
            coreData.pkgCstConfigControl = msrPkgCstConfigControl->getPkgCstConfigControlData(cpu);

        msrDev->closeMsrFd(cpu);
        packet.intelData->coreData.append(coreData);
    }

    void IntelCPU::fillThreadData(const int cpu, const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const {
        PWTS::Intel::IntelThreadData thdData {};

        if (!features.contains(PWTS::Feature::INTEL_CPU_GROUP)) {
            packet.intelData->threadData.append(thdData);
            return;
        }

        if (!msrDev->openMsrFd(cpu)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to open msr fd for cpu %1").arg(cpu));

            packet.errors.insert(PWTS::DError::NO_MSR_FD);
            packet.intelData->threadData.append(thdData);
            return;
        }

        if (features.contains(PWTS::Feature::INTEL_HWP_GROUP)) {
            thdData.hwpCapapabilities = ia32HWPCapabilities->getHWPCapabilitiesData(cpu);
            thdData.hwpRequest = ia32HWPRequest->getHWPRequestData(cpu);
        }

        msrDev->closeMsrFd(cpu);
        packet.intelData->threadData.append(thdData);
    }

    void IntelCPU::fillDaemonPacket(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, PWTS::DaemonPacket &packet) const {
        packet.intelData = QSharedPointer<PWTS::Intel::IntelData>::create();

        if (cpuInfo->numCores != coreIdxList.size()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("core index count mismatch: %1 / %2").arg(coreIdxList.size()).arg(cpuInfo->numCores));

            packet.errors.insert(PWTS::DError::CORE_IDX_MISMATCH);
            return;
        }

        fillPackageData(features, packet);

        for (int i=0,l=cpuInfo->numCores; i<l; ++i)
            fillCoreData(coreIdxList[i], features, packet);

        for (int i=0,l=cpuInfo->numLogicalCpus; i<l; ++i)
            fillThreadData(i, features, packet);

        if (!mchbar.isNull())
            mchbar->fillPacketData(features, packet);
    }

    void IntelCPU::applyPackageSettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::INTEL_CPU_GROUP))
            return;

        if (!msrDev->openMsrFd(0)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("failed to open msr fd"));

            errors.insert(PWTS::DError::NO_MSR_FD);
            return;
        }

        const QSharedPointer<PWTS::Intel::IntelData> data = packet.intelData;
        const bool hasTurboRatioLimit = features.contains(PWTS::Feature::INTEL_TURBO_RATIO_LIMIT) &&
                                        features.contains(PWTS::Feature::INTEL_TURBO_RATIO_LIMIT_RW);
        const bool hasTurboPowCurrentLimit = features.contains(PWTS::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT) &&
                                             features.contains(PWTS::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT_RW);

        if (features.contains(PWTS::Feature::INTEL_VR_CURRENT_CFG) && !msrVrCurrentConfig->setVrCurrentConfig(data->vrCurrentCfg))
            errors.insert(PWTS::DError::W_VR_CURRENT_CFG);

        if (features.contains(PWTS::Feature::INTEL_PP1_CURRENT_CFG) && !msrPP1CurrentConfig->setPP1CurrentConfig(data->pp1CurrentCfg))
            errors.insert(PWTS::DError::W_PP1_CURRENT_CFG);

        if (features.contains(PWTS::Feature::INTEL_CPU_POWER_BALANCE) && !msrPP0Policy->setPP0Priority(data->pp0Priority))
            errors.insert(PWTS::DError::W_CPU_BLNC);

        if (features.contains(PWTS::Feature::INTEL_GPU_POWER_BALANCE) && !msrPP1Policy->setPP1Priority(data->pp1Priority))
            errors.insert(PWTS::DError::W_GPU_BLNC);

        if (features.contains(PWTS::Feature::INTEL_ENERGY_PERF_BIAS) && !ia32EnergyPerfBias->setPowerPolicyPreference(data->energyPerfBias))
            errors.insert(PWTS::DError::W_ENERGY_PERF_BIAS);

        if (hasTurboRatioLimit && !msrTurboRatioLimit->setTurboRatioLimit(data->turboRatioLimit))
            errors.insert(PWTS::DError::W_TURBO_RATIO_LIMIT);

        if (features.contains(PWTS::Feature::INTEL_IA32_MISC_ENABLE_GROUP) && !ia32MiscEnable->setMiscProcessorFeatures(data->miscProcFeatures))
            errors.insert(PWTS::DError::W_MISC_PROC_FEATURES);

        if (features.contains(PWTS::Feature::INTEL_POWER_CTL) && !msrPowerCtl->setPowerCtl(data->powerCtl))
            errors.insert(PWTS::DError::W_POWER_CTL);

        if (features.contains(PWTS::Feature::INTEL_MISC_PWR_MGMT) && !msrMiscPwrMgmt->setMiscPwrMgmt(data->miscPwrMgmt))
            errors.insert(PWTS::DError::W_MISC_PWR_MGMT);

        if (features.contains(PWTS::Feature::INTEL_UNDERVOLT_GROUP)) {
            const MSR_UNK_FIVR_CONTROL::FIVRWriteResult res = msrUnkFivrControl->setFIVRControl(data->undervoltData);

            if (data->undervoltData.isValid()) // don't save invalid data
                fivr = data->undervoltData.getValue();

            if (features.contains(PWTS::Feature::INTEL_UNDERVOLT_CPU) && !res.cpu)
                errors.insert(PWTS::DError::W_CPU_UV);

            if (features.contains(PWTS::Feature::INTEL_UNDERVOLT_GPU) && !res.gpu)
                errors.insert(PWTS::DError::W_GPU_UV);

            if (features.contains(PWTS::Feature::INTEL_UNDERVOLT_CACHE) && !res.cpuCache)
                errors.insert(PWTS::DError::W_CACHE_UV);

            if (features.contains(PWTS::Feature::INTEL_UNDERVOLT_UNSLICE) && !res.unslice)
                errors.insert(PWTS::DError::W_UNSLICE_UV);

            if (features.contains(PWTS::Feature::INTEL_UNDERVOLT_SYSAGENT) && !res.sysAgent)
                errors.insert(PWTS::DError::W_SA_UV);
        }

        if (hasTurboPowCurrentLimit && !msrTurboPowerCurrentLimit->setTurboPowerCurrentLimit(data->turboPowerCurrentLimit))
            errors.insert(PWTS::DError::W_TURBO_POWER_CURRENT_LIMIT);

        if (features.contains(PWTS::Feature::INTEL_PKG_POWER_LIMIT) && !msrPkgPowerLimit->setPkgPowerLimit(data->pkgPowerLimit, regsCache->raplPowerUnit))
            errors.insert(PWTS::DError::W_PKG_POWER_LIMIT);

        if (features.contains(PWTS::Feature::INTEL_HWP_GROUP)) {
            if (ia32PmEnable->getHWPEnableBit().getValue() == 0 && !ia32PmEnable->setHWPEnableBit(data->hwpEnable))
                errors.insert(PWTS::DError::W_HWP_ENABLE);

            if (features.contains(PWTS::Feature::INTEL_HWP_REQ_PKG) && !ia32HWPRequestPkg->setHWPRequestPkg(data->hwpRequestPkg))
                errors.insert(PWTS::DError::W_HWP_REQ_PKG);

            if (features.contains(PWTS::Feature::INTEL_HWP_CTL) && !ia32HwpCtl->setHWPCtlBit(data->hwpPkgCtlPolarity))
                errors.insert(PWTS::DError::W_HWP_CTL);
        }

        msrDev->closeMsrFd(0);
    }

    void IntelCPU::applyCoreSettings(const int cpu, const int coreIdx, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::INTEL_CPU_GROUP))
            return;

        const PWTS::Intel::IntelCoreData &data = packet.intelData->coreData[cpu];

        if (!msrDev->openMsrFd(coreIdx)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to open msr fd for cpu %1").arg(coreIdx));

            return;
        }

        if (features.contains(PWTS::Feature::INTEL_PKG_CST_CONFIG_CONTROL) && !msrPkgCstConfigControl->setPkgCstConfigControlData(coreIdx, data.pkgCstConfigControl))
            errors.insert(PWTS::DError::W_PKG_CST_CONFIG_CONTROL);

        msrDev->closeMsrFd(cpu);
    }

    void IntelCPU::applyThreadSettings(const int cpu, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::INTEL_CPU_GROUP))
            return;

        if (!msrDev->openMsrFd(cpu)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to open msr fd for cpu %1").arg(cpu));

            return;
        }

        const PWTS::Intel::IntelThreadData &data = packet.intelData->threadData[cpu];

        if (features.contains(PWTS::Feature::INTEL_HWP_GROUP) && !ia32HWPRequest->setHWPRequest(cpu, data.hwpRequest))
            errors.insert(PWTS::DError::W_HWP_REQ);

        msrDev->closeMsrFd(cpu);
    }

    QSet<PWTS::DError> IntelCPU::applySettings(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const {
        const QSharedPointer<PWTS::Intel::IntelData> idata = packet.intelData;
        QSet<PWTS::DError> errors;

        if (idata.isNull()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write("empty data");

            return errors;

        } else if (cpuInfo->numCores != idata->coreData.size()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("core count mismatch: %1 / %2").arg(cpuInfo->numCores).arg(idata->coreData.size()));

            errors.insert(PWTS::DError::CORE_DATA_MISMATCH);
            return errors;

        } else if (cpuInfo->numLogicalCpus != idata->threadData.size()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("thread count mismatch: %1 / %2").arg(cpuInfo->numLogicalCpus).arg(idata->threadData.size()));

            errors.insert(PWTS::DError::THREAD_DATA_MISMATCH);
            return errors;

        } else if (cpuInfo->numCores != coreIdxList.size()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("core index count mismatch: %1 / %2").arg(coreIdxList.size()).arg(cpuInfo->numCores));

            errors.insert(PWTS::DError::CORE_IDX_MISMATCH);
            return errors;
        }

        applyPackageSettings(features, packet, errors);

        for (int i=0,l=cpuInfo->numCores; i<l; ++i)
            applyCoreSettings(i, coreIdxList[i], features, packet, errors);

        for (int i=0,l=cpuInfo->numLogicalCpus; i<l; ++i)
            applyThreadSettings(i, features, packet, errors);

        if (!mchbar.isNull())
            errors.unite(mchbar->applySettings(features, packet));

        return errors;
    }

    PWTS::ROData<int> IntelCPU::getTemperature() const {
        if (ia32PackageThermStatus.isNull() || msrTemperatureTarget.isNull())
            return {};

        if (!msrDev->openMsrFd(0)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to open msr fd"));

            return {};
        }

        const PWTS::ROData<PWTS::Intel::PkgThermalStatusInfo> pkgThermInfo = ia32PackageThermStatus->getPkgThermStatusData();

        msrDev->closeMsrFd(0);

        if (!pkgThermInfo.isValid() || !regsCache->temperatureTarget.isValid())
            return {};

        return PWTS::ROData<int>(regsCache->temperatureTarget.getValue() - pkgThermInfo.getValue().digitalReadout, true);
    }
}
