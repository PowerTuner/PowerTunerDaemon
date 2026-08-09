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
        mchbar = std::make_unique<MCHBAR>(cpuID->x86.family);

        if (!mchbar->init(cpuID->x86.family, cpuID->x86.ext_model))
            mchbar.reset();

        setupCPUModelRegisters();

        ia32MiscEnable = std::make_unique<IA32_MISC_ENABLE>();
        msrTurboRatioLimit = std::make_unique<MSR_TURBO_RATIO_LIMIT>();

        if (hasIA32PkgThermStatusBit())
            ia32PackageThermStatus = std::make_unique<IA32_PACKAGE_THERM_STATUS>();

        if (hasEnergyPerfBiasBit())
            ia32EnergyPerfBias = std::make_unique<IA32_ENERGY_PERF_BIAS>();

        if (hasHWPBit()) {
            ia32PmEnable = std::make_unique<IA32_PM_ENABLE>();
            ia32HWPCapabilities = std::make_unique<IA32_HWP_CAPABILITIES>();
            ia32HWPRequest = std::make_unique<IA32_HWP_REQUEST>();

            if (hasHWPRequestPkgBit())
                ia32HWPRequestPkg = std::make_unique<IA32_HWP_REQUEST_PKG>();

            if (hasHWPCtlBit())
                ia32HwpCtl = std::make_unique<IA32_HWP_CTL>();
        }

        buildRegistersCache();
    }

    void IntelCPU::setupCPUModelRegisters() {
        switch (cpuInfo->family) {
            case Family6: {
                switch (cpuInfo->extModel) {
                    case Clarkdale: {
                        msrMiscPwrMgmt = std::make_unique<MSR_MISC_PWR_MGMT_NHLM>();
                        msrPlatformInfo = std::make_unique<MSR_PLATFORM_INFO_NHLM>();
                        msrTurboPowerCurrentLimit = std::make_unique<MSR_TURBO_POWER_CURRENT_LIMIT>();
                        msrPowerCtl = std::make_unique<MSR_POWER_CTL_NHLM>();
                        msrTemperatureTarget = std::make_unique<MSR_TEMPERATURE_TARGET_NHLM>();
                    }
                        break;
                    case SandyBridge: {
                        msrMiscPwrMgmt = std::make_unique<MSR_MISC_PWR_MGMT_NHLM>();
                        msrPlatformInfo = std::make_unique<MSR_PLATFORM_INFO_NHLM>();
                        msrPkgPowerLimit = std::make_unique<MSR_PKG_POWER_LIMIT>();
                        msrVrCurrentConfig = std::make_unique<MSR_VR_CURRENT_CONFIG_SB>();
                        msrPP0Policy = std::make_unique<MSR_PP0_POLICY>();
                        msrPP1Policy = std::make_unique<MSR_PP1_POLICY>();
                        msrPP1CurrentConfig = std::make_unique<MSR_PP1_CURRENT_CONFIG>();
                        msrPkgCstConfigControl = std::make_unique<MSR_PKG_CST_CONFIG_CONTROL_SB>();
                        msrPowerCtl = std::make_unique<MSR_POWER_CTL_SB>();
                        msrTemperatureTarget = std::make_unique<MSR_TEMPERATURE_TARGET_NHLM>();
                    }
                        break;
                    case IvyBridge: {
                        msrMiscPwrMgmt = std::make_unique<MSR_MISC_PWR_MGMT_NHLM>();
                        msrPlatformInfo = std::make_unique<MSR_PLATFORM_INFO_NHLM>();
                        msrPkgPowerLimit = std::make_unique<MSR_PKG_POWER_LIMIT>();
                        msrVrCurrentConfig = std::make_unique<MSR_VR_CURRENT_CONFIG_SB>();
                        msrPP0Policy = std::make_unique<MSR_PP0_POLICY>();
                        msrPP1Policy = std::make_unique<MSR_PP1_POLICY>();
                        msrPP1CurrentConfig = std::make_unique<MSR_PP1_CURRENT_CONFIG>();
                        msrPkgCstConfigControl = std::make_unique<MSR_PKG_CST_CONFIG_CONTROL_SB>();
                        msrPowerCtl = std::make_unique<MSR_POWER_CTL_SB>();
                        msrTemperatureTarget = std::make_unique<MSR_TEMPERATURE_TARGET_NHLM>();
                    }
                        break;
                    case IceLakeU: {
                        msrMiscPwrMgmt = std::make_unique<MSR_MISC_PWR_MGMT_NHLM>();
                        msrPlatformInfo = std::make_unique<MSR_PLATFORM_INFO_NHLM>();
                        msrPkgPowerLimit = std::make_unique<MSR_PKG_POWER_LIMIT>();
                        msrVrCurrentConfig = std::make_unique<MSR_VR_CURRENT_CONFIG_SB>();
                        msrPP0Policy = std::make_unique<MSR_PP0_POLICY>();
                        msrPP1Policy = std::make_unique<MSR_PP1_POLICY>();
                        msrPowerCtl = std::make_unique<MSR_POWER_CTL_SB>();
                        msrUnkFivrControl = std::make_unique<MSR_UNK_FIVR_CONTROL_ICL>();
                        msrTemperatureTarget = std::make_unique<MSR_TEMPERATURE_TARGET_NHLM>();
                    }
                        break;
                    case TigerLakeU: {
                        msrMiscPwrMgmt = std::make_unique<MSR_MISC_PWR_MGMT_NHLM>();
                        msrPlatformInfo = std::make_unique<MSR_PLATFORM_INFO_NHLM>();
                        msrPkgPowerLimit = std::make_unique<MSR_PKG_POWER_LIMIT>();
                        msrVrCurrentConfig = std::make_unique<MSR_VR_CURRENT_CONFIG_SB>();
                        msrPP0Policy = std::make_unique<MSR_PP0_POLICY>();
                        msrPP1Policy = std::make_unique<MSR_PP1_POLICY>();
                        msrPowerCtl = std::make_unique<MSR_POWER_CTL_SB>();
                        msrTemperatureTarget = std::make_unique<MSR_TEMPERATURE_TARGET_NHLM>();
                    }
                        break;
                    case AlderLakeN: {
                        msrPlatformInfo = std::make_unique<MSR_PLATFORM_INFO_NHLM>();
                        msrPkgPowerLimit = std::make_unique<MSR_PKG_POWER_LIMIT>();
                        msrVrCurrentConfig = std::make_unique<MSR_VR_CURRENT_CONFIG_SB>();
                        msrPP0Policy = std::make_unique<MSR_PP0_POLICY>();
                        msrPP1Policy = std::make_unique<MSR_PP1_POLICY>();
                        msrPowerCtl = std::make_unique<MSR_POWER_CTL_SB>();
                        msrTemperatureTarget = std::make_unique<MSR_TEMPERATURE_TARGET_NHLM>();
                    }
                        break;
                    case LunarLake: {
                        msrPlatformInfo = std::make_unique<MSR_PLATFORM_INFO_NHLM>();
                        msrPkgPowerLimit = std::make_unique<MSR_PKG_POWER_LIMIT>();
                        msrVrCurrentConfig = std::make_unique<MSR_VR_CURRENT_CONFIG_CU1>();
                        msrPP0Policy = std::make_unique<MSR_PP0_POLICY>();
                        msrPP1Policy = std::make_unique<MSR_PP1_POLICY>();
                        msrPowerCtl = std::make_unique<MSR_POWER_CTL_CU1>();
                        msrPkgCstConfigControl = std::make_unique<MSR_PKG_CST_CONFIG_CONTROL_CU1>();
                        msrTemperatureTarget = std::make_unique<MSR_TEMPERATURE_TARGET_NHLM>();
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

    void IntelCPU::buildRegistersCache() const {
        regsCache = std::make_unique<RegistersCache>();

        if (msrDev->openMsrFd(0)) {
            if (msrPkgPowerLimit)
                regsCache->raplPowerUnit = MSR_RAPL_POWER_UNIT().get();

            if (msrTemperatureTarget) {
                const PWTS::RWData<PWTS::Intel::TemperatureTarget> tempTarget = msrTemperatureTarget->get();

                if (tempTarget.isValid())
                    regsCache->temperatureTarget = PWTS::ROData<int>(tempTarget.getValue().temperatureTarget, true);
            }

            msrDev->closeMsrFd(0);

        } else if (logger.isLevel(PWTS::LogLevel::Error)) {
            logger.write("failed to create registers cache");
        }
    }

    bool IntelCPU::hasIA32PkgThermStatusBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(6, 6, eax) == 1;
    }

    bool IntelCPU::hasEnergyPerfBiasBit() const {
        const uint32_t ecx = cpuidRaw->basic_cpuid[6][cpu_registers_t::ECX];

        return getBitfield(3, 3, ecx) == 1;
    }

    // eax bit is cleared when disabled, so show the feature even when disable is 1
    bool IntelCPU::hasTurboBoostTechBit() const {
        const PWTS::RWData<PWTS::Intel::MiscProcFeatures> miscFeaturesData = ia32MiscEnable->get();
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];
        const int disableTurbo = !miscFeaturesData.isValid() ? 0 : miscFeaturesData.getValue().disableTurboMode;

        return getBitfield(1, 1, eax) == 1 || disableTurbo == 1;
    }

    bool IntelCPU::hasEnhancedSpeedStepBit() const {
        const uint32_t ecx = cpuidRaw->basic_cpuid[1][cpu_registers_t::ECX];

        return getBitfield(7, 7, ecx) == 1;
    }

    bool IntelCPU::hasHWPBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(7, 7, eax) == 1;
    }

    bool IntelCPU::hasHWPReqActivityWindowBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(9, 9, eax) == 1;
    }

    bool IntelCPU::hasHWPReqEPPBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(10, 10, eax) == 1;
    }

    bool IntelCPU::hasHWPRequestPkgBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(11, 11, eax) == 1;
    }

    bool IntelCPU::hasHWPReqValidBitsBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(17, 17, eax) == 1;
    }

    bool IntelCPU::hasHWPCtlBit() const {
        const uint32_t eax = cpuidRaw->basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(22, 22, eax) == 1;
    }

    QSet<PWTS::Feature> IntelCPU::getFeatures() const {
        if (!msrDev->openMsrFd(0)) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QStringLiteral("failed to open msr fd"));

            return {};
        }

        QSet<PWTS::Feature> features;
        std::optional<MSR_PLATFORM_INFO::Info> platformInfo;

        if (msrPlatformInfo) {
            platformInfo = msrPlatformInfo->get();

            features.unite({PWTS::Feature::INTEL_PLATFORM_INFO, PWTS::Feature::INTEL_CPU_GROUP});

            if (dynamic_cast<MSR_PLATFORM_INFO_NHLM *>(msrPlatformInfo.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_PLATFORM_INFO_NHLM);
        }

        if (ia32PackageThermStatus)
            features.unite({PWTS::Feature::INTEL_PKG_THERM_STATUS, PWTS::Feature::INTEL_CPU_STAT_GROUP});

        if (msrPkgPowerLimit)
            features.unite({PWTS::Feature::INTEL_PKG_POWER_LIMIT, PWTS::Feature::INTEL_CPU_GROUP});

        if (msrVrCurrentConfig)
            features.unite({PWTS::Feature::INTEL_VR_CURRENT_CFG, PWTS::Feature::INTEL_CPU_GROUP});

        if (msrPP1CurrentConfig)
            features.unite({PWTS::Feature::INTEL_PP1_CURRENT_CFG, PWTS::Feature::INTEL_CPU_GROUP});

        if (msrTurboPowerCurrentLimit) {
            features.unite({PWTS::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT, PWTS::Feature::INTEL_CPU_GROUP});

            if (platformInfo && platformInfo->programmableTDPLimitForTurboMode)
                features.insert(PWTS::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT_RW);
        }

        if (msrPP0Policy)
            features.unite({PWTS::Feature::INTEL_CPU_POWER_BALANCE, PWTS::Feature::INTEL_CPU_GROUP});

        if (msrPP1Policy)
            features.unite({PWTS::Feature::INTEL_GPU_POWER_BALANCE, PWTS::Feature::INTEL_CPU_GROUP});

        if (ia32EnergyPerfBias)
            features.unite({PWTS::Feature::INTEL_ENERGY_PERF_BIAS, PWTS::Feature::INTEL_CPU_GROUP});

        if (msrTurboRatioLimit) {
            features.unite({PWTS::Feature::INTEL_TURBO_RATIO_LIMIT, PWTS::Feature::INTEL_CPU_GROUP});

            if (platformInfo && platformInfo->programmableRatioLimitForTurboMode)
                features.insert(PWTS::Feature::INTEL_TURBO_RATIO_LIMIT_RW);
        }

        if (hasTurboBoostTechBit())
            features.unite({PWTS::Feature::INTEL_TURBO_BOOST, PWTS::Feature::INTEL_CPU_GROUP, PWTS::Feature::INTEL_IA32_MISC_ENABLE_GROUP});

        if (hasEnhancedSpeedStepBit())
            features.unite({PWTS::Feature::INTEL_ENHANCED_SPEEDSTEP, PWTS::Feature::INTEL_CPU_GROUP, PWTS::Feature::INTEL_IA32_MISC_ENABLE_GROUP});

        if (msrPowerCtl) {
            features.unite({PWTS::Feature::INTEL_POWER_CTL, PWTS::Feature::INTEL_CPU_GROUP});

            if (dynamic_cast<MSR_POWER_CTL_NHLM *>(msrPowerCtl.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_POWER_CTL_NHLM);
            else if (dynamic_cast<MSR_POWER_CTL_SB *>(msrPowerCtl.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_POWER_CTL_SB);
            else if (dynamic_cast<MSR_POWER_CTL_CU1 *>(msrPowerCtl.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_POWER_CTL_CU1);
        }

        if (msrMiscPwrMgmt) {
            features.unite({PWTS::Feature::INTEL_MISC_PWR_MGMT, PWTS::Feature::INTEL_CPU_GROUP});

            if (dynamic_cast<MSR_MISC_PWR_MGMT_NHLM *>(msrMiscPwrMgmt.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_MISC_PWR_MGMT_NHLM);
        }

        if (msrUnkFivrControl) {
            const PWTS::ROData<MSR_UNK_FIVR_CONTROL::FIVRCapabilities> fivrCaps = msrUnkFivrControl->getCapabilities();

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

        if (ia32PmEnable) {
            features.insert(PWTS::Feature::INTEL_HWP_GROUP);

            if (ia32HWPRequestPkg)
                features.insert(PWTS::Feature::INTEL_HWP_REQ_PKG);

            if (hasHWPReqEPPBit())
                features.insert(PWTS::Feature::INTEL_HWP_EPP);

            if (hasHWPReqActivityWindowBit())
                features.insert(PWTS::Feature::INTEL_HWP_ACT_WIND);

            if (hasHWPReqValidBitsBit())
                features.insert(PWTS::Feature::INTEL_HWP_VALID_BITS);

            if (ia32HwpCtl)
                features.insert(PWTS::Feature::INTEL_HWP_CTL);
        }

        if (msrPkgCstConfigControl) {
            features.unite({PWTS::Feature::INTEL_PKG_CST_CONFIG_CONTROL, PWTS::Feature::INTEL_CPU_GROUP});

            if (dynamic_cast<MSR_PKG_CST_CONFIG_CONTROL_SB *>(msrPkgCstConfigControl.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_PKG_CST_CONFIG_CONTROL_SB);
            else if (dynamic_cast<MSR_PKG_CST_CONFIG_CONTROL_CU1 *>(msrPkgCstConfigControl.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_PKG_CST_CONFIG_CONTROL_CU1);
        }

        if (msrTemperatureTarget)
            features.unite({PWTS::Feature::INTEL_TEMPERATURE_TARGET, PWTS::Feature::INTEL_CPU_GROUP});

        msrDev->closeMsrFd(0);

        if (mchbar)
            features.unite(mchbar->getFeatures());

        return features;
    }

    void IntelCPU::fillPackageData(const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::INTEL_CPU_GROUP))
            return;

        if (!msrDev->openMsrFd(0)) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QStringLiteral("failed to open msr fd"));

            packet.errors.insert(PWTS::DError::NO_MSR_FD);
            return;
        }

        if (features.contains(PWTS::Feature::INTEL_PKG_POWER_LIMIT))
            packet.intelData->pkgPowerLimit = msrPkgPowerLimit->get(regsCache->raplPowerUnit);

        if (features.contains(PWTS::Feature::INTEL_VR_CURRENT_CFG))
            packet.intelData->vrCurrentCfg = msrVrCurrentConfig->get();

        if (features.contains(PWTS::Feature::INTEL_PP1_CURRENT_CFG))
            packet.intelData->pp1CurrentCfg = msrPP1CurrentConfig->get();

        if (features.contains(PWTS::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT))
            packet.intelData->turboPowerCurrentLimit = msrTurboPowerCurrentLimit->get();

        if (features.contains(PWTS::Feature::INTEL_CPU_POWER_BALANCE))
            packet.intelData->pp0Priority = msrPP0Policy->get();

        if (features.contains(PWTS::Feature::INTEL_GPU_POWER_BALANCE))
            packet.intelData->pp1Priority = msrPP1Policy->get();

        if (features.contains(PWTS::Feature::INTEL_ENERGY_PERF_BIAS))
            packet.intelData->energyPerfBias = ia32EnergyPerfBias->get();

        if (features.contains(PWTS::Feature::INTEL_TURBO_RATIO_LIMIT))
            packet.intelData->turboRatioLimit = msrTurboRatioLimit->get();

        if (features.contains(PWTS::Feature::INTEL_IA32_MISC_ENABLE_GROUP))
            packet.intelData->miscProcFeatures = ia32MiscEnable->get();

        if (features.contains(PWTS::Feature::INTEL_POWER_CTL))
            packet.intelData->powerCtl = msrPowerCtl->get();

        if (features.contains(PWTS::Feature::INTEL_MISC_PWR_MGMT))
            packet.intelData->miscPwrMgmt = msrMiscPwrMgmt->get();

        if (features.contains(PWTS::Feature::INTEL_UNDERVOLT_GROUP))
            packet.intelData->undervoltData = PWTS::RWData<PWTS::Intel::FIVRControlUV>(fivr, true);

        if (features.contains(PWTS::Feature::INTEL_HWP_GROUP)) {
            packet.intelData->hwpEnable = ia32PmEnable->get();

            if (features.contains(PWTS::Feature::INTEL_HWP_REQ_PKG))
                packet.intelData->hwpRequestPkg = ia32HWPRequestPkg->get();

            if (features.contains(PWTS::Feature::INTEL_HWP_CTL))
                packet.intelData->hwpPkgCtlPolarity = ia32HwpCtl->get();
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
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("failed to open msr fd for cpu %1").arg(cpu));

            packet.errors.insert(PWTS::DError::NO_MSR_FD);
            packet.intelData->coreData.append(coreData);
            return;
        }

        if (features.contains(PWTS::Feature::INTEL_PKG_CST_CONFIG_CONTROL))
            coreData.pkgCstConfigControl = msrPkgCstConfigControl->get(cpu);

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
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("failed to open msr fd for cpu %1").arg(cpu));

            packet.errors.insert(PWTS::DError::NO_MSR_FD);
            packet.intelData->threadData.append(thdData);
            return;
        }

        if (features.contains(PWTS::Feature::INTEL_HWP_GROUP)) {
            thdData.hwpCapapabilities = ia32HWPCapabilities->get(cpu);
            thdData.hwpRequest = ia32HWPRequest->get(cpu);
        }

        msrDev->closeMsrFd(cpu);
        packet.intelData->threadData.append(thdData);
    }

    void IntelCPU::fillDaemonPacket(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, PWTS::DaemonPacket &packet) const {
        packet.intelData = QSharedPointer<PWTS::Intel::IntelData>::create();

        if (cpuInfo->numCores != coreIdxList.size()) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("core index count mismatch: %1 / %2").arg(coreIdxList.size()).arg(cpuInfo->numCores));

            packet.errors.insert(PWTS::DError::CORE_IDX_MISMATCH);
            return;
        }

        fillPackageData(features, packet);

        for (int i=0,l=cpuInfo->numCores; i<l; ++i)
            fillCoreData(coreIdxList[i], features, packet);

        for (int i=0,l=cpuInfo->numLogicalCpus; i<l; ++i)
            fillThreadData(i, features, packet);

        if (mchbar)
            mchbar->fillPacketData(features, packet);
    }

    void IntelCPU::applyPackageSettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::INTEL_CPU_GROUP))
            return;

        if (!msrDev->openMsrFd(0)) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QStringLiteral("failed to open msr fd"));

            errors.insert(PWTS::DError::NO_MSR_FD);
            return;
        }

        const QSharedPointer<PWTS::Intel::IntelData> data = packet.intelData;
        const bool hasTurboRatioLimit = features.contains(PWTS::Feature::INTEL_TURBO_RATIO_LIMIT) &&
                                        features.contains(PWTS::Feature::INTEL_TURBO_RATIO_LIMIT_RW);
        const bool hasTurboPowCurrentLimit = features.contains(PWTS::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT) &&
                                             features.contains(PWTS::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT_RW);

        if (features.contains(PWTS::Feature::INTEL_VR_CURRENT_CFG) && !msrVrCurrentConfig->set(data->vrCurrentCfg))
            errors.insert(PWTS::DError::W_VR_CURRENT_CFG);

        if (features.contains(PWTS::Feature::INTEL_PP1_CURRENT_CFG) && !msrPP1CurrentConfig->set(data->pp1CurrentCfg))
            errors.insert(PWTS::DError::W_PP1_CURRENT_CFG);

        if (features.contains(PWTS::Feature::INTEL_CPU_POWER_BALANCE) && !msrPP0Policy->set(data->pp0Priority))
            errors.insert(PWTS::DError::W_CPU_BLNC);

        if (features.contains(PWTS::Feature::INTEL_GPU_POWER_BALANCE) && !msrPP1Policy->set(data->pp1Priority))
            errors.insert(PWTS::DError::W_GPU_BLNC);

        if (features.contains(PWTS::Feature::INTEL_ENERGY_PERF_BIAS) && !ia32EnergyPerfBias->set(data->energyPerfBias))
            errors.insert(PWTS::DError::W_ENERGY_PERF_BIAS);

        if (hasTurboRatioLimit && !msrTurboRatioLimit->set(data->turboRatioLimit))
            errors.insert(PWTS::DError::W_TURBO_RATIO_LIMIT);

        if (features.contains(PWTS::Feature::INTEL_IA32_MISC_ENABLE_GROUP) && !ia32MiscEnable->set(data->miscProcFeatures))
            errors.insert(PWTS::DError::W_MISC_PROC_FEATURES);

        if (features.contains(PWTS::Feature::INTEL_POWER_CTL) && !msrPowerCtl->set(data->powerCtl))
            errors.insert(PWTS::DError::W_POWER_CTL);

        if (features.contains(PWTS::Feature::INTEL_MISC_PWR_MGMT) && !msrMiscPwrMgmt->set(data->miscPwrMgmt))
            errors.insert(PWTS::DError::W_MISC_PWR_MGMT);

        if (features.contains(PWTS::Feature::INTEL_UNDERVOLT_GROUP)) {
            const MSR_UNK_FIVR_CONTROL::FIVRWriteResult res = msrUnkFivrControl->set(data->undervoltData);

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

        if (hasTurboPowCurrentLimit && !msrTurboPowerCurrentLimit->set(data->turboPowerCurrentLimit))
            errors.insert(PWTS::DError::W_TURBO_POWER_CURRENT_LIMIT);

        if (features.contains(PWTS::Feature::INTEL_PKG_POWER_LIMIT) && !msrPkgPowerLimit->set(data->pkgPowerLimit, regsCache->raplPowerUnit))
            errors.insert(PWTS::DError::W_PKG_POWER_LIMIT);

        if (features.contains(PWTS::Feature::INTEL_HWP_GROUP)) {
            if (ia32PmEnable->get().getValue() == 0 && !ia32PmEnable->set(data->hwpEnable))
                errors.insert(PWTS::DError::W_HWP_ENABLE);

            if (features.contains(PWTS::Feature::INTEL_HWP_REQ_PKG) && !ia32HWPRequestPkg->set(data->hwpRequestPkg))
                errors.insert(PWTS::DError::W_HWP_REQ_PKG);

            if (features.contains(PWTS::Feature::INTEL_HWP_CTL) && !ia32HwpCtl->set(data->hwpPkgCtlPolarity))
                errors.insert(PWTS::DError::W_HWP_CTL);
        }

        msrDev->closeMsrFd(0);
    }

    void IntelCPU::applyCoreSettings(const int cpu, const int coreIdx, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::INTEL_CPU_GROUP))
            return;

        const PWTS::Intel::IntelCoreData &data = packet.intelData->coreData[cpu];

        if (!msrDev->openMsrFd(coreIdx)) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("failed to open msr fd for cpu %1").arg(coreIdx));

            return;
        }

        if (features.contains(PWTS::Feature::INTEL_PKG_CST_CONFIG_CONTROL) && !msrPkgCstConfigControl->set(coreIdx, data.pkgCstConfigControl))
            errors.insert(PWTS::DError::W_PKG_CST_CONFIG_CONTROL);

        msrDev->closeMsrFd(cpu);
    }

    void IntelCPU::applyThreadSettings(const int cpu, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::INTEL_CPU_GROUP))
            return;

        if (!msrDev->openMsrFd(cpu)) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("failed to open msr fd for cpu %1").arg(cpu));

            return;
        }

        const PWTS::Intel::IntelThreadData &data = packet.intelData->threadData[cpu];

        if (features.contains(PWTS::Feature::INTEL_HWP_GROUP) && !ia32HWPRequest->set(cpu, data.hwpRequest))
            errors.insert(PWTS::DError::W_HWP_REQ);

        msrDev->closeMsrFd(cpu);
    }

    QSet<PWTS::DError> IntelCPU::applySettings(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const {
        const QSharedPointer<PWTS::Intel::IntelData> idata = packet.intelData;
        QSet<PWTS::DError> errors;

        if (idata.isNull()) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write("empty data");

            return errors;

        } else if (cpuInfo->numCores != idata->coreData.size()) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("core count mismatch: %1 / %2").arg(cpuInfo->numCores).arg(idata->coreData.size()));

            errors.insert(PWTS::DError::CORE_DATA_MISMATCH);
            return errors;

        } else if (cpuInfo->numLogicalCpus != idata->threadData.size()) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("thread count mismatch: %1 / %2").arg(cpuInfo->numLogicalCpus).arg(idata->threadData.size()));

            errors.insert(PWTS::DError::THREAD_DATA_MISMATCH);
            return errors;

        } else if (cpuInfo->numCores != coreIdxList.size()) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("core index count mismatch: %1 / %2").arg(coreIdxList.size()).arg(cpuInfo->numCores));

            errors.insert(PWTS::DError::CORE_IDX_MISMATCH);
            return errors;
        }

        applyPackageSettings(features, packet, errors);

        for (int i=0,l=cpuInfo->numCores; i<l; ++i)
            applyCoreSettings(i, coreIdxList[i], features, packet, errors);

        for (int i=0,l=cpuInfo->numLogicalCpus; i<l; ++i)
            applyThreadSettings(i, features, packet, errors);

        if (mchbar)
            errors.unite(mchbar->applySettings(features, packet));

        return errors;
    }

    PWTS::ROData<int> IntelCPU::getTemperature() const {
        if (!ia32PackageThermStatus || !msrTemperatureTarget)
            return {};

        if (!msrDev->openMsrFd(0)) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("failed to open msr fd"));

            return {};
        }

        const std::optional<PWTS::Intel::PkgThermalStatusInfo> pkgThermStatusI = ia32PackageThermStatus->get();

        msrDev->closeMsrFd(0);

        if (!pkgThermStatusI || !regsCache->temperatureTarget.isValid())
            return {};

        return PWTS::ROData<int>(regsCache->temperatureTarget.getValue() - pkgThermStatusI->digitalReadout, true);
    }
}
