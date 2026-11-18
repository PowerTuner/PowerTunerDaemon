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
#include "AMDCPU.h"

namespace PWTD::AMD {
    using namespace Qt::StringLiterals;

    AMDCPU::AMDCPU(const cpu_raw_data_t &cpuidRaw, LIBPWT::DeviceData &devData) {
        std::unordered_set<LIBPWT::Feature> &features = devData.daemonInfo->features.cpu;

        devData.cpu = LIBPWT::CPUVendor::AMD;
        devData.amdMsr = LIBPWT::AMD::MSR{
            .cores = std::vector<LIBPWT::AMD::MSRCore>(devData.cpuInfo->cores),
            .threads = std::vector<LIBPWT::AMD::MSRThread>(devData.cpuInfo->threads)
        };
        ryzenAdj = std::make_unique<RyzenAdj>(devData);

        if (!ryzenAdj->isRunning())
            ryzenAdj.reset();

        if (hasHWPStateBit(cpuidRaw)) {
            features.insert(LIBPWT::Feature::AMD_HWPSTATE);
            msrPStateCurrentLimit = std::make_unique<MSR_PSTATE_CURRENT_LIMIT>();
            msrPStateControl = std::make_unique<MSR_PSTATE_CONTROL>();
        }

        if (hasCorePerformanceBoostBit(cpuidRaw)) {
            features.insert(LIBPWT::Feature::AMD_CORE_PERFORMANCE_BOOST);
            msrCorePerformanceBoost = std::make_unique<MSR_CORE_PERFORMANCE_BOOST>();
        }

        if (hasCPPCBit(cpuidRaw)) {
            features.insert(LIBPWT::Feature::AMD_CPPC);
            msrCppcCapability1 = std::make_unique<MSR_CPPC_CAPABILITY_1>();
            msrCppcEnable = std::make_unique<MSR_CPPC_ENABLE>();
            msrCppcRequest = std::make_unique<MSR_CPPC_REQUEST>();
        }
    }

    bool AMDCPU::hasHWPStateBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t edx = cpuidRaw.ext_cpuid[7][cpu_registers_t::EDX];

        return getBitfield(7, 7, edx) == 1;
    }
    bool AMDCPU::hasCorePerformanceBoostBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t edx = cpuidRaw.ext_cpuid[7][cpu_registers_t::EDX];

        return getBitfield(9, 9, edx) == 1;
    }

    bool AMDCPU::hasCPPCBit(const cpu_raw_data_t &cpuidRaw) const {
        const uint32_t ebx = cpuidRaw.ext_cpuid[8][cpu_registers_t::EBX];

        return getBitfield(27, 27, ebx) == 1;
    }

    void AMDCPU::readThread(const int cpu, LIBPWT::DeviceData &devData) const {
        LIBPWT::AMD::MSRThread &thd = devData.amdMsr->threads[cpu];

        if (!msrDev->openFd(cpu)) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to open msr fd for cpu %1"_s.arg(cpu));

            return devData.errors.push_back(LIBPWT::Error::NO_MSR_FD);
        }

        if (msrCppcCapability1 && msrCppcRequest) {
            thd.cppcCapability1 = msrCppcCapability1->get(cpu);
            thd.cppcRequest = msrCppcRequest->get(cpu);

            if (!thd.cppcCapability1)
                devData.errors.push_back(LIBPWT::Error::R_AMD_CPPC_CAP1);

            if (!thd.cppcRequest)
                devData.errors.push_back(LIBPWT::Error::R_AMD_CPPC_REQ);
        }

        if (msrPStateControl) {
            thd.pstateCmd = msrPStateControl->get(cpu);

            if (!thd.pstateCmd)
                devData.errors.push_back(LIBPWT::Error::R_AMD_HWPSTATE_CMD);
        }

        if (msrCorePerformanceBoost) {
            thd.corePerfBoost = msrCorePerformanceBoost->get(cpu);

            if (!thd.corePerfBoost)
                devData.errors.push_back(LIBPWT::Error::R_AMD_CORE_PERFORMANCE_BOOST);
        }

        msrDev->closeFd(cpu);
    }

    void AMDCPU::read(LIBPWT::DeviceData &devData) const {
        if (ryzenAdj)
            ryzenAdj->read(devData);

        if (!msrDev->openFd(0))
            return devData.errors.push_back(LIBPWT::Error::NO_MSR_FD);

        if (msrPStateCurrentLimit) {
            devData.amdMsr->pstateCurrentLimit = msrPStateCurrentLimit->get();

            if (!devData.amdMsr->pstateCurrentLimit)
                devData.errors.push_back(LIBPWT::Error::R_AMD_PSTATE_CUR_LIMIT);
        }

        if (msrCppcEnable) {
            devData.amdMsr->cppcEnableBit = msrCppcEnable->get();

            if (!devData.amdMsr->cppcEnableBit)
                devData.errors.push_back(LIBPWT::Error::R_AMD_CPPC_ENBL_BIT);
        }

        msrDev->closeFd(0);

        for (int i=0,l=devData.cpuInfo->threads; i<l; ++i)
            readThread(i, devData);
    }

    void AMDCPU::writeThread(const int cpu, const LIBPWT::DeviceData &clientData, std::vector<LIBPWT::Error> &errors) const {
        const LIBPWT::AMD::MSRThread &thread = clientData.amdMsr->threads[cpu];

        if (!msrDev->openFd(cpu)) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to open msr fd for cpu %1"_s.arg(cpu));
            return;
        }

        if (msrPStateControl && thread.pstateCmd && !msrPStateControl->set(cpu, *thread.pstateCmd))
            errors.push_back(LIBPWT::Error::W_AMD_HWPSTATE_CMD);

        if (msrCorePerformanceBoost && thread.corePerfBoost && !msrCorePerformanceBoost->set(cpu, *thread.corePerfBoost))
            errors.push_back(LIBPWT::Error::W_AMD_CORE_PERFORMANCE_BOOST);

        if (msrCppcRequest && thread.cppcRequest && !msrCppcRequest->set(cpu, *thread.cppcRequest))
            errors.push_back(LIBPWT::Error::W_AMD_CPPC_REQ);

        msrDev->closeFd(cpu);
    }

    std::vector<LIBPWT::Error> AMDCPU::write(LIBPWT::DeviceData &devData, const LIBPWT::DeviceData &clientData) const {
        std::vector<LIBPWT::Error> errors;

        if (ryzenAdj)
            errors.append_range(ryzenAdj->write(devData, clientData));

        if (!msrDev->openFd(0)) {
            errors.push_back(LIBPWT::Error::NO_MSR_FD);
            return errors;
        }

        if (msrCppcEnable && clientData.amdMsr->cppcEnableBit) {
            const std::optional<int> enableBit = msrCppcEnable->get();

            if (enableBit && *enableBit == 0 && !msrCppcEnable->set(*clientData.amdMsr->cppcEnableBit))
                errors.push_back(LIBPWT::Error::W_AMD_CPPC_ENBL_BIT);
        }

        msrDev->closeFd(0);

        for (int i=0,l=devData.cpuInfo->threads; i<l; ++i)
            writeThread(i, clientData, errors);

        return errors;
    }

    std::optional<int> AMDCPU::getTemperature() const {
        return ryzenAdj->getTemperature();
    }
}
