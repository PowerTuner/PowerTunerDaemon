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
    using namespace Qt::StringLiterals;

    IntelCPU::IntelCPU(const cpu_id_t &cpuid, const cpu_raw_data_t &cpuidRaw, LIBPWT::DeviceData &devData) {
        devData.cpu = LIBPWT::CPUVendor::Intel;
        devData.intelMsr = LIBPWT::Intel::MSR{
            .cores = std::vector<LIBPWT::Intel::MSRCore>(devData.cpuInfo->cores),
            .threads = std::vector<LIBPWT::Intel::MSRThread>(devData.cpuInfo->threads)
        };
        mchbar = std::make_unique<MCHBAR>(cpuid.x86.family, cpuid.x86.ext_model, devData);

        if (!mchbar->isAccessible())
            mchbar.reset();

        setupCPUModelRegisters(devData.cpuInfo->family, devData.cpuInfo->extModel);

        ia32MiscEnable = std::make_unique<IA32_MISC_ENABLE>();
        msrTurboRatioLimit = std::make_unique<MSR_TURBO_RATIO_LIMIT>();

        if (hasIA32PkgThermStatusBit(cpuidRaw))
            ia32PackageThermStatus = std::make_unique<IA32_PACKAGE_THERM_STATUS>();

        if (hasEnergyPerfBiasBit(cpuidRaw))
            ia32EnergyPerfBias = std::make_unique<IA32_ENERGY_PERF_BIAS>();

        if (hasHWPBit(cpuidRaw)) {
            ia32PmEnable = std::make_unique<IA32_PM_ENABLE>();
            ia32HWPCapabilities = std::make_unique<IA32_HWP_CAPABILITIES>();
            ia32HWPRequest = std::make_unique<IA32_HWP_REQUEST>();

            if (hasHWPRequestPkgBit(cpuidRaw))
                ia32HWPRequestPkg = std::make_unique<IA32_HWP_REQUEST_PKG>();

            if (hasHWPCtlBit(cpuidRaw))
                ia32HwpCtl = std::make_unique<IA32_HWP_CTL>();
        }

        setFeatures(cpuidRaw, devData);
    }

    void IntelCPU::setupCPUModelRegisters(const int family, const int extModel) {
        switch (family) {
            case Family6: {
                switch (extModel) {
                    case Clarkdale: {
                        msrMiscPwrMgmt = std::make_unique<MSR_MISC_PWR_MGMT_NHLM>();
                        msrPlatformInfo = std::make_unique<MSR_PLATFORM_INFO_NHLM>();
                        msrTurboPowerCurrentLimit = std::make_unique<MSR_TURBO_POWER_CURRENT_LIMIT>();
                        msrPowerCtl = std::make_unique<MSR_POWER_CTL_NHLM>();
                        msrTemperatureTarget = std::make_unique<MSR_TEMPERATURE_TARGET_NHLM>();
                    }
                        break;
                    case SandyBridge:
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
                        msrFivrControl = std::make_unique<MSR_UNK_FIVR_CONTROL_ICL>();
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

    bool IntelCPU::hasIA32PkgThermStatusBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t eax = cpuidRaw.basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(6, 6, eax) == 1;
    }

    bool IntelCPU::hasEnergyPerfBiasBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t ecx = cpuidRaw.basic_cpuid[6][cpu_registers_t::ECX];

        return getBitfield(3, 3, ecx) == 1;
    }

    // eax bit is cleared when disabled, so show the feature even when disable is 1
    bool IntelCPU::hasTurboBoostTechBit(const cpu_raw_data_t &cpuidRaw) const {
        const std::optional<LIBPWT::Intel::MiscProcFeatures> miscFeaturesData = ia32MiscEnable->get();
        const uint32_t eax = cpuidRaw.basic_cpuid[6][cpu_registers_t::EAX];
        const int disableTurbo = !miscFeaturesData ? 0 : miscFeaturesData->disableTurboMode;

        return getBitfield(1, 1, eax) == 1 || disableTurbo == 1;
    }

    bool IntelCPU::hasEnhancedSpeedStepBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t ecx = cpuidRaw.basic_cpuid[1][cpu_registers_t::ECX];

        return getBitfield(7, 7, ecx) == 1;
    }

    bool IntelCPU::hasHWPBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t eax = cpuidRaw.basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(7, 7, eax) == 1;
    }

    bool IntelCPU::hasHWPReqActivityWindowBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t eax = cpuidRaw.basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(9, 9, eax) == 1;
    }

    bool IntelCPU::hasHWPReqEPPBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t eax = cpuidRaw.basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(10, 10, eax) == 1;
    }

    bool IntelCPU::hasHWPRequestPkgBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t eax = cpuidRaw.basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(11, 11, eax) == 1;
    }

    bool IntelCPU::hasHWPReqValidBitsBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t eax = cpuidRaw.basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(17, 17, eax) == 1;
    }

    bool IntelCPU::hasHWPCtlBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t eax = cpuidRaw.basic_cpuid[6][cpu_registers_t::EAX];

        return getBitfield(22, 22, eax) == 1;
    }

    void IntelCPU::setFeatures(const cpu_raw_data_t &cpuidRaw, LIBPWT::DeviceData &devData) {
        std::unordered_set<LIBPWT::Feature> &features = devData.daemonInfo->features.cpu;
        std::optional<MSR_PLATFORM_INFO::Info> platformInfo;

        if (!msrDev->openFd(0)) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to open msr fd"_s);
            return;
        }

        if (msrPlatformInfo) {
            features.insert(LIBPWT::Feature::INTEL_PLATFORM_INFO);
            platformInfo = msrPlatformInfo->get();

            if (dynamic_cast<MSR_PLATFORM_INFO_NHLM *>(msrPlatformInfo.get()) != nullptr)
                features.insert(LIBPWT::Feature::INTEL_PLATFORM_INFO_NHLM);
        }

        if (ia32PackageThermStatus)
            features.insert(LIBPWT::Feature::INTEL_PKG_THERM_STATUS);

        if (msrPkgPowerLimit) {
            const std::optional<MSR_RAPL_POWER_UNIT::Units> pu = MSR_RAPL_POWER_UNIT().get();

            if (pu) {
                features.insert(LIBPWT::Feature::INTEL_PKG_POWER_LIMIT);
                powerUnits = *pu;

            } else {
                msrPkgPowerLimit.reset();
            }
        }

        if (msrVrCurrentConfig)
            features.insert(LIBPWT::Feature::INTEL_VR_CURRENT_CFG);

        if (msrPP1CurrentConfig)
            features.insert(LIBPWT::Feature::INTEL_PP1_CURRENT_CFG);

        if (msrTurboPowerCurrentLimit) {
            features.insert(LIBPWT::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT);

            if (platformInfo && platformInfo->programmableTDPLimitForTurboMode)
                features.insert(LIBPWT::Feature::INTEL_TURBO_POWER_CURRENT_LIMIT_RW);
        }

        if (msrPP0Policy)
            features.insert(LIBPWT::Feature::INTEL_CPU_POWER_BALANCE);

        if (msrPP1Policy)
            features.insert(LIBPWT::Feature::INTEL_GPU_POWER_BALANCE);

        if (ia32EnergyPerfBias)
            features.insert(LIBPWT::Feature::INTEL_ENERGY_PERF_BIAS);

        if (msrTurboRatioLimit) {
            features.insert(LIBPWT::Feature::INTEL_TURBO_RATIO_LIMIT);

            if (platformInfo && platformInfo->programmableRatioLimitForTurboMode)
                features.insert(LIBPWT::Feature::INTEL_TURBO_RATIO_LIMIT_RW);
        }

        if (hasTurboBoostTechBit(cpuidRaw))
            features.insert(LIBPWT::Feature::INTEL_TURBO_BOOST);

        if (hasEnhancedSpeedStepBit(cpuidRaw))
            features.insert(LIBPWT::Feature::INTEL_ENHANCED_SPEEDSTEP);

        if (msrPowerCtl) {
            features.insert(LIBPWT::Feature::INTEL_POWER_CTL);

            if (dynamic_cast<MSR_POWER_CTL_NHLM *>(msrPowerCtl.get()) != nullptr)
                features.insert(LIBPWT::Feature::INTEL_POWER_CTL_NHLM);
            else if (dynamic_cast<MSR_POWER_CTL_SB *>(msrPowerCtl.get()) != nullptr)
                features.insert(LIBPWT::Feature::INTEL_POWER_CTL_SB);
            else if (dynamic_cast<MSR_POWER_CTL_CU1 *>(msrPowerCtl.get()) != nullptr)
                features.insert(LIBPWT::Feature::INTEL_POWER_CTL_CU1);
        }

        if (msrMiscPwrMgmt) {
            features.insert(LIBPWT::Feature::INTEL_MISC_PWR_MGMT);

            if (dynamic_cast<MSR_MISC_PWR_MGMT_NHLM *>(msrMiscPwrMgmt.get()) != nullptr)
                features.insert(LIBPWT::Feature::INTEL_MISC_PWR_MGMT_NHLM);
        }

        if (msrFivrControl) {
            if (msrFivrControl->hasCPU())
                features.insert(LIBPWT::Feature::INTEL_UNDERVOLT_CPU);

            if (msrFivrControl->hasGPU())
                features.insert(LIBPWT::Feature::INTEL_UNDERVOLT_GPU);

            if (msrFivrControl->hasCPUCache())
                features.insert(LIBPWT::Feature::INTEL_UNDERVOLT_CACHE);

            if (msrFivrControl->hasUnslice())
                features.insert(LIBPWT::Feature::INTEL_UNDERVOLT_UNSLICE);

            if (msrFivrControl->hasSysAgent())
                features.insert(LIBPWT::Feature::INTEL_UNDERVOLT_SYSAGENT);
        }

        if (ia32PmEnable) {
            features.insert(LIBPWT::Feature::INTEL_HWP);

            if (ia32HWPRequestPkg)
                features.insert(LIBPWT::Feature::INTEL_HWP_REQ_PKG);

            if (hasHWPReqEPPBit(cpuidRaw))
                features.insert(LIBPWT::Feature::INTEL_HWP_EPP);

            if (hasHWPReqActivityWindowBit(cpuidRaw))
                features.insert(LIBPWT::Feature::INTEL_HWP_ACT_WIND);

            if (hasHWPReqValidBitsBit(cpuidRaw))
                features.insert(LIBPWT::Feature::INTEL_HWP_VALID_BITS);

            if (ia32HwpCtl)
                features.insert(LIBPWT::Feature::INTEL_HWP_CTL);
        }

        if (msrPkgCstConfigControl) {
            features.insert(LIBPWT::Feature::INTEL_PKG_CST_CONFIG_CONTROL);

            if (dynamic_cast<MSR_PKG_CST_CONFIG_CONTROL_SB *>(msrPkgCstConfigControl.get()) != nullptr)
                features.insert(LIBPWT::Feature::INTEL_PKG_CST_CONFIG_CONTROL_SB);
            else if (dynamic_cast<MSR_PKG_CST_CONFIG_CONTROL_CU1 *>(msrPkgCstConfigControl.get()) != nullptr)
                features.insert(LIBPWT::Feature::INTEL_PKG_CST_CONFIG_CONTROL_CU1);
        }

        if (msrTemperatureTarget) {
            const std::optional<LIBPWT::Intel::TemperatureTarget> tempTarget = msrTemperatureTarget->get();

            if (tempTarget) {
                features.insert(LIBPWT::Feature::INTEL_TEMPERATURE_TARGET);
                baseTCCTemperatureTarget = tempTarget->temperatureTarget;

            } else {
                msrTemperatureTarget.reset();
            }
        }

        msrDev->closeFd(0);
    }

    void IntelCPU::readCore(const int cpu, LIBPWT::DeviceData &devData) const {
        LIBPWT::Intel::MSRCore &core = devData.intelMsr->cores[cpu];

        if (!msrDev->openFd(cpu)) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to open msr fd for cpu %1"_s.arg(cpu));

            return devData.errors.push_back(LIBPWT::Error::NO_MSR_FD);
        }

        if (msrPkgCstConfigControl) {
            core.pkgCstConfigControl = msrPkgCstConfigControl->get(cpu);

            if (!core.pkgCstConfigControl)
                devData.errors.push_back(LIBPWT::Error::R_PKG_CST_CONFIG_CONTROL);
        }

        msrDev->closeFd(cpu);
    }

    void IntelCPU::readThread(const int cpu, LIBPWT::DeviceData &devData) const {
        LIBPWT::Intel::MSRThread &thd = devData.intelMsr->threads[cpu];

        if (!msrDev->openFd(cpu)) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to open msr fd for cpu %1"_s.arg(cpu));

            return devData.errors.push_back(LIBPWT::Error::NO_MSR_FD);
        }

        if (ia32HWPCapabilities) {
            thd.hwpCapapabilities = ia32HWPCapabilities->get(cpu);

            if (!thd.hwpCapapabilities)
                devData.errors.push_back(LIBPWT::Error::R_HWP_CAPABILITIES);
        }

        if (ia32HWPRequest) {
            thd.hwpRequest = ia32HWPRequest->get(cpu);

            if (!thd.hwpRequest)
                devData.errors.push_back(LIBPWT::Error::R_HWP_REQ);
        }

        msrDev->closeFd(cpu);
    }

    void IntelCPU::read(LIBPWT::DeviceData &devData) const {
        if (!msrDev->openFd(0))
            return devData.errors.push_back(LIBPWT::Error::NO_MSR_FD);

        if (ia32MiscEnable) {
            devData.intelMsr->miscProcFeatures = ia32MiscEnable->get();

            if (!devData.intelMsr->miscProcFeatures)
                devData.errors.push_back(LIBPWT::Error::R_MISC_PROC_FEATURES);
        }

        if (msrPkgPowerLimit) {
            devData.intelMsr->pkgPowerLimit = msrPkgPowerLimit->get(powerUnits);

            if (!devData.intelMsr->pkgPowerLimit)
                devData.errors.push_back(LIBPWT::Error::R_PKG_POWER_LIMIT);
        }

        if (msrVrCurrentConfig) {
            devData.intelMsr->vrCurrentCfg = msrVrCurrentConfig->get();

            if (!devData.intelMsr->vrCurrentCfg)
                devData.errors.push_back(LIBPWT::Error::R_VR_CURRENT_CFG);
        }

        if (msrPP1CurrentConfig) {
            devData.intelMsr->pp1CurrentCfg = msrPP1CurrentConfig->get();

            if (!devData.intelMsr->pp1CurrentCfg)
                devData.errors.push_back(LIBPWT::Error::R_PP1_CURRENT_CFG);
        }

        if (msrTurboPowerCurrentLimit) {
            devData.intelMsr->turboPowerCurrentLimit = msrTurboPowerCurrentLimit->get();

            if (!devData.intelMsr->turboPowerCurrentLimit)
                devData.errors.push_back(LIBPWT::Error::R_TURBO_POWER_CURRENT_LIMIT);
        }

        if (msrPP0Policy) {
            devData.intelMsr->pp0Priority = msrPP0Policy->get();

            if (!devData.intelMsr->pp0Priority)
                devData.errors.push_back(LIBPWT::Error::R_CPU_BLNC);
        }

        if (msrPP1Policy) {
            devData.intelMsr->pp1Priority = msrPP1Policy->get();

            if (!devData.intelMsr->pp1Priority)
                devData.errors.push_back(LIBPWT::Error::R_GPU_BLNC);
        }

        if (ia32EnergyPerfBias) {
            devData.intelMsr->energyPerfBias = ia32EnergyPerfBias->get();

            if (!devData.intelMsr->energyPerfBias)
                devData.errors.push_back(LIBPWT::Error::R_ENERGY_PERF_BIAS);
        }

        if (msrTurboRatioLimit) {
            devData.intelMsr->turboRatioLimit = msrTurboRatioLimit->get();

            if (!devData.intelMsr->turboRatioLimit)
                devData.errors.push_back(LIBPWT::Error::R_TURBO_RATIO_LIMIT);
        }

        if (msrPowerCtl) {
            devData.intelMsr->powerCtl = msrPowerCtl->get();

            if (!devData.intelMsr->powerCtl)
                devData.errors.push_back(LIBPWT::Error::R_POWER_CTL);
        }

        if (msrMiscPwrMgmt) {
            devData.intelMsr->miscPwrMgmt = msrMiscPwrMgmt->get();

            if (!devData.intelMsr->miscPwrMgmt)
                devData.errors.push_back(LIBPWT::Error::R_MISC_PWR_MGMT);
        }

        if (ia32PmEnable) {
            devData.intelMsr->hwpEnable = ia32PmEnable->get();

            if (!devData.intelMsr->hwpEnable)
                devData.errors.push_back(LIBPWT::Error::R_HWP_ENABLE);
        }

        if (ia32HWPRequestPkg) {
            devData.intelMsr->hwpRequestPkg = ia32HWPRequestPkg->get();

            if (!devData.intelMsr->hwpRequestPkg)
                devData.errors.push_back(LIBPWT::Error::R_HWP_REQ_PKG);
        }

        if (ia32HwpCtl) {
            devData.intelMsr->hwpPkgCtlPolarity = ia32HwpCtl->get();

            if (!devData.intelMsr->hwpPkgCtlPolarity)
                devData.errors.push_back(LIBPWT::Error::R_HWP_CTL);
        }

        msrDev->closeFd(0);

        for (const int core: devData.systemInfo->coreIDs)
            readCore(core, devData);

        for (int i=0,l=devData.cpuInfo->threads; i<l; ++i)
            readThread(i, devData);

        if (mchbar)
            mchbar->read(devData);
    }

    void IntelCPU::writeCore(const int cpu, const LIBPWT::DeviceData &clientData, std::vector<LIBPWT::Error> &errors) const {
        const LIBPWT::Intel::MSRCore &core = clientData.intelMsr->cores[cpu];

        if (!msrDev->openFd(cpu)) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to open msr fd for cpu %1"_s.arg(cpu));
            return;
        }

        if (msrPkgCstConfigControl && core.pkgCstConfigControl && !msrPkgCstConfigControl->set(cpu, *core.pkgCstConfigControl))
            errors.push_back(LIBPWT::Error::W_PKG_CST_CONFIG_CONTROL);

        msrDev->closeFd(cpu);
    }

    void IntelCPU::writeThread(const int cpu, const LIBPWT::DeviceData &clientData, std::vector<LIBPWT::Error> &errors) const {
        const LIBPWT::Intel::MSRThread &thd = clientData.intelMsr->threads[cpu];

        if (!msrDev->openFd(cpu)) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to open msr fd for cpu %1"_s.arg(cpu));
            return;
        }

        if (ia32HWPRequest && thd.hwpRequest && !ia32HWPRequest->set(cpu, *thd.hwpRequest))
            errors.push_back(LIBPWT::Error::W_HWP_REQ);

        msrDev->closeFd(cpu);
    }

    std::vector<LIBPWT::Error> IntelCPU::write(LIBPWT::DeviceData &devData, const LIBPWT::DeviceData &clientData) const {
        std::vector<LIBPWT::Error> errors;

        if (!msrDev->openFd(0)) {
            errors.push_back(LIBPWT::Error::NO_MSR_FD);
            return errors;
        }

        if (msrVrCurrentConfig && clientData.intelMsr->vrCurrentCfg && !msrVrCurrentConfig->set(*clientData.intelMsr->vrCurrentCfg))
            errors.push_back(LIBPWT::Error::W_VR_CURRENT_CFG);

        if (msrPP1CurrentConfig && clientData.intelMsr->pp1CurrentCfg && !msrPP1CurrentConfig->set(*clientData.intelMsr->pp1CurrentCfg))
            errors.push_back(LIBPWT::Error::W_PP1_CURRENT_CFG);

        if (msrPP0Policy && clientData.intelMsr->pp0Priority && !msrPP0Policy->set(*clientData.intelMsr->pp0Priority))
            errors.push_back(LIBPWT::Error::W_CPU_BLNC);

        if (msrPP1Policy && clientData.intelMsr->pp1Priority && !msrPP1Policy->set(*clientData.intelMsr->pp1Priority))
            errors.push_back(LIBPWT::Error::W_GPU_BLNC);

        if (ia32EnergyPerfBias && clientData.intelMsr->energyPerfBias && !ia32EnergyPerfBias->set(*clientData.intelMsr->energyPerfBias))
            errors.push_back(LIBPWT::Error::W_ENERGY_PERF_BIAS);

        if (msrTurboRatioLimit && clientData.intelMsr->turboRatioLimit && !msrTurboRatioLimit->set(*clientData.intelMsr->turboRatioLimit))
            errors.push_back(LIBPWT::Error::W_TURBO_RATIO_LIMIT);

        if (ia32MiscEnable && clientData.intelMsr->miscProcFeatures && !ia32MiscEnable->set(*clientData.intelMsr->miscProcFeatures))
            errors.push_back(LIBPWT::Error::W_MISC_PROC_FEATURES);

        if (msrPowerCtl && clientData.intelMsr->powerCtl && !msrPowerCtl->set(*clientData.intelMsr->powerCtl))
            errors.push_back(LIBPWT::Error::W_POWER_CTL);

        if (msrMiscPwrMgmt && clientData.intelMsr->miscPwrMgmt && !msrMiscPwrMgmt->set(*clientData.intelMsr->miscPwrMgmt))
            errors.push_back(LIBPWT::Error::W_MISC_PWR_MGMT);

        if (msrFivrControl && clientData.intelMsr->fivrData) {
            const std::unordered_set<LIBPWT::Feature> &features = devData.daemonInfo->features.cpu;
            const LIBPWT::Intel::FIVRControl uv = *clientData.intelMsr->fivrData;

            devData.intelMsr->fivrData = uv;

            if (features.contains(LIBPWT::Feature::INTEL_UNDERVOLT_CPU) && !msrFivrControl->setCPU(uv))
                errors.push_back(LIBPWT::Error::W_CPU_UV);

            if (features.contains(LIBPWT::Feature::INTEL_UNDERVOLT_GPU) && !msrFivrControl->setGPU(uv))
                errors.push_back(LIBPWT::Error::W_GPU_UV);

            if (features.contains(LIBPWT::Feature::INTEL_UNDERVOLT_CACHE) && !msrFivrControl->setCPUCache(uv))
                errors.push_back(LIBPWT::Error::W_CACHE_UV);

            if (features.contains(LIBPWT::Feature::INTEL_UNDERVOLT_UNSLICE) && !msrFivrControl->setUnslice(uv))
                errors.push_back(LIBPWT::Error::W_UNSLICE_UV);

            if (features.contains(LIBPWT::Feature::INTEL_UNDERVOLT_SYSAGENT) && !msrFivrControl->setSysAgent(uv))
                errors.push_back(LIBPWT::Error::W_SA_UV);
        }

        if (msrTurboPowerCurrentLimit && clientData.intelMsr->turboPowerCurrentLimit && !msrTurboPowerCurrentLimit->set(*clientData.intelMsr->turboPowerCurrentLimit))
            errors.push_back(LIBPWT::Error::W_TURBO_POWER_CURRENT_LIMIT);

        if (msrPkgPowerLimit && clientData.intelMsr->pkgPowerLimit && !msrPkgPowerLimit->set(*clientData.intelMsr->pkgPowerLimit, powerUnits))
            errors.push_back(LIBPWT::Error::W_PKG_POWER_LIMIT);

        if (ia32PmEnable && clientData.intelMsr->hwpEnable) {
            const std::optional<int> enableBit = ia32PmEnable->get();

            if (enableBit && *enableBit == 0 && !ia32PmEnable->set(*clientData.intelMsr->hwpEnable))
                errors.push_back(LIBPWT::Error::W_HWP_ENABLE);
        }

        if (ia32HWPRequestPkg && !ia32HWPRequestPkg->set(*clientData.intelMsr->hwpRequestPkg))
            errors.push_back(LIBPWT::Error::W_HWP_REQ_PKG);

        if (ia32HwpCtl && !ia32HwpCtl->set(*clientData.intelMsr->hwpPkgCtlPolarity))
            errors.push_back(LIBPWT::Error::W_HWP_CTL);

        msrDev->closeFd(0);

        for (const int core: devData.systemInfo->coreIDs)
            writeCore(core, clientData, errors);

        for (int i=0,l=devData.cpuInfo->threads; i<l; ++i)
            writeThread(i, clientData, errors);

        if (mchbar)
            errors.append_range(mchbar->write(clientData));

        return errors;
    }

    std::optional<int> IntelCPU::getTemperature() const {
        if (!ia32PackageThermStatus || !msrTemperatureTarget)
            return {};

        if (!msrDev->openFd(0)) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to open msr fd"_s);
            return {};
        }

        const std::optional<LIBPWT::Intel::PkgThermalStatusInfo> pkgThermStatusI = ia32PackageThermStatus->get();

        msrDev->closeFd(0);

        if (!pkgThermStatusI) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to read thermal status info"_s);
            return {};
        }

        return baseTCCTemperatureTarget - pkgThermStatusI->digitalReadout;
    }
}
