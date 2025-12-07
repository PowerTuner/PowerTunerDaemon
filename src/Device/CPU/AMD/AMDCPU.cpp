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
    AMDCPU::AMDCPU(const QSharedPointer<cpu_id_t> &cpuID, const QSharedPointer<cpu_raw_data_t> &cpuRawData): CPUDevice(cpuID, cpuRawData) {
        cpuInfo->vendor = PWTS::CPUVendor::AMD;
        msrDev = MSRFactory::getMSRInstance();

        ryzenAdj.reset(new RyzenAdj);
        if (!ryzenAdj->init(cpuID->num_cores))
            ryzenAdj.reset();

        if (hasHWPStateBit()) {
            msrPStateCurrentLimit.reset(new MSR_PSTATE_CURRENT_LIMIT);
            msrPStateControl.reset(new MSR_PSTATE_CONTROL);
        }

        if (hasCorePerformanceBoostBit())
            msrCorePerformanceBoost.reset(new MSR_CORE_PERFORMANCE_BOOST);

        if (hasCPPCBit()) {
            msrCppcCapability1.reset(new MSR_CPPC_CAPABILITY_1);
            msrCppcEnable.reset(new MSR_CPPC_ENABLE);
            msrCppcRequest.reset(new MSR_CPPC_REQUEST);
        }
    }

    bool AMDCPU::hasHWPStateBit() const {
        const uint32_t edx = cpuidRaw->ext_cpuid[7][cpu_registers_t::EDX];
        bool ret;

        try {
            ret = getBitfield(7, 7, edx) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    bool AMDCPU::hasCorePerformanceBoostBit() const {
        const uint32_t edx = cpuidRaw->ext_cpuid[7][cpu_registers_t::EDX];
        bool ret;

        try {
            ret = getBitfield(9, 9, edx) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    bool AMDCPU::hasCPPCBit() const {
        const uint32_t ebx = cpuidRaw->ext_cpuid[8][cpu_registers_t::EBX];
        bool ret;

        try {
            ret = getBitfield(27, 27, ebx) == 1;

        } catch ([[maybe_unused]] std::invalid_argument const &e) {
            return false;
        }

        return ret;
    }

    QSet<PWTS::Feature> AMDCPU::getFeatures() const {
        if (!msrDev->openMsrFd(0)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("failed to open msr fd"));

            return {};
        }

        QSet<PWTS::Feature> features;

        if (!msrPStateCurrentLimit.isNull())
            features.unite({PWTS::Feature::AMD_HWPSTATE, PWTS::Feature::AMD_CPU_GROUP});

        if (!msrCorePerformanceBoost.isNull())
            features.unite({PWTS::Feature::AMD_CORE_PERFORMANCE_BOOST, PWTS::Feature::AMD_CPU_GROUP});

        if (!msrCppcEnable.isNull())
            features.unite({PWTS::Feature::AMD_CPPC, PWTS::Feature::AMD_CPU_GROUP});

        msrDev->closeMsrFd(0);

        if (!ryzenAdj.isNull())
            features.unite(ryzenAdj->getFeatures());

        return features;
    }

    void AMDCPU::fillPackageData(const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::AMD_CPU_GROUP))
            return;

        if (!msrDev->openMsrFd(0)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("failed to open msr fd"));

            packet.errors.insert(PWTS::DError::NO_MSR_FD);
            return;
        }

        if (features.contains(PWTS::Feature::AMD_HWPSTATE))
            packet.amdData->pstateCurrentLimit = msrPStateCurrentLimit->getPStateCurrentLimitData();

        if (features.contains(PWTS::Feature::AMD_CPPC))
            packet.amdData->cppcEnableBit = msrCppcEnable->getCPPCEnableBit();

        msrDev->closeMsrFd(0);
    }

    void AMDCPU::fillCoreData(const int cpu, const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const {
        PWTS::AMD::AMDCoreData coreData {};

        packet.amdData->coreData.append(coreData);
    }

    void AMDCPU::fillThreadData(const int cpu, const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const {
        PWTS::AMD::AMDThreadData thdData {};

        if (!features.contains(PWTS::Feature::AMD_CPU_GROUP)) {
            packet.amdData->threadData.append(thdData);
            return;
        }

        if (!msrDev->openMsrFd(cpu)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to open msr fd for cpu %1").arg(cpu));

            packet.errors.insert(PWTS::DError::NO_MSR_FD);
            packet.amdData->threadData.append(thdData);
            return;
        }

        if (features.contains(PWTS::Feature::AMD_CPPC)) {
            thdData.cppcCapability1 = msrCppcCapability1->getCPPCCapability1Data(cpu);
            thdData.cppcRequest = msrCppcRequest->getCPPCRequestData(cpu);
        }

        if (features.contains(PWTS::Feature::AMD_HWPSTATE))
            thdData.pstateCmd = msrPStateControl->getPStateControlData(cpu);

        if (features.contains(PWTS::Feature::AMD_CORE_PERFORMANCE_BOOST))
            thdData.corePerfBoost = msrCorePerformanceBoost->getCorePerformanceBoostData(cpu);

        msrDev->closeMsrFd(cpu);
        packet.amdData->threadData.append(thdData);
    }

    void AMDCPU::fillDaemonPacket(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, PWTS::DaemonPacket &packet) const {
        packet.amdData = QSharedPointer<PWTS::AMD::AMDData>::create();

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

        if (!ryzenAdj.isNull())
            ryzenAdj->fillPacketData(features, packet);
    }

    void AMDCPU::applyPackageSettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::AMD_CPU_GROUP))
            return;

        if (!msrDev->openMsrFd(0)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("failed to open msr fd"));

            errors.insert(PWTS::DError::NO_MSR_FD);
            return;
        }

        const QSharedPointer<PWTS::AMD::AMDData> data = packet.amdData;

        if (features.contains(PWTS::Feature::AMD_CPPC)) {
            if (msrCppcEnable->getCPPCEnableBit().getValue() == 0 && !msrCppcEnable->setCPPCEnableBit(data->cppcEnableBit))
                errors.insert(PWTS::DError::W_AMD_CPPC_ENBL_BIT);
        }

        msrDev->closeMsrFd(0);
    }

    void AMDCPU::applyThreadSettings(const int cpu, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::AMD_CPU_GROUP))
            return;

        if (!msrDev->openMsrFd(cpu)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to open msr fd for cpu %1").arg(cpu));

            return;
        }

        const PWTS::AMD::AMDThreadData &data = packet.amdData->threadData[cpu];

        if (features.contains(PWTS::Feature::AMD_HWPSTATE) && !msrPStateControl->setPStateControl(cpu, data.pstateCmd))
            errors.insert(PWTS::DError::W_AMD_HWPSTATE_CMD);

        if (features.contains(PWTS::Feature::AMD_CORE_PERFORMANCE_BOOST) && !msrCorePerformanceBoost->setCorePerformanceBoost(cpu, data.corePerfBoost))
            errors.insert(PWTS::DError::W_AMD_CORE_PERFORMANCE_BOOST);

        if (features.contains(PWTS::Feature::AMD_CPPC) && !msrCppcRequest->setCPPCRequest(cpu, data.cppcRequest))
            errors.insert(PWTS::DError::W_AMD_CPPC_REQ);

        msrDev->closeMsrFd(cpu);
    }

    QSet<PWTS::DError> AMDCPU::applySettings(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const {
        const QSharedPointer<PWTS::AMD::AMDData> adata = packet.amdData;
        QSet<PWTS::DError> errors;

        if (adata.isNull()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write("empty data");

            return {};

        } else if (cpuInfo->numCores != adata->coreData.size()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("core count mismatch: %1 / %2").arg(cpuInfo->numCores).arg(adata->coreData.size()));

            errors.insert(PWTS::DError::CORE_DATA_MISMATCH);
            return errors;

        } else if (cpuInfo->numLogicalCpus != adata->threadData.size()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("thread count mismatch: %1 / %2").arg(cpuInfo->numLogicalCpus).arg(adata->threadData.size()));

            errors.insert(PWTS::DError::THREAD_DATA_MISMATCH);
            return errors;

        } else if (cpuInfo->numCores != coreIdxList.size()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("core index count mismatch: %1 / %2").arg(coreIdxList.size()).arg(cpuInfo->numCores));

            errors.insert(PWTS::DError::CORE_IDX_MISMATCH);
            return errors;
        }

        applyPackageSettings(features, packet, errors);

        for (int i=0,l=cpuInfo->numLogicalCpus; i<l; ++i)
            applyThreadSettings(i, features, packet, errors);

        if (!ryzenAdj.isNull())
            errors.unite(ryzenAdj->applySettings(features, coreIdxList, packet));

        return errors;
    }

    PWTS::ROData<int> AMDCPU::getTemperature() const {
        return ryzenAdj->getTemperature();
    }
}
