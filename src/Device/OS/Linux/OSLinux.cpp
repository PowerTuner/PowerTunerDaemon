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
extern "C" {
#include <pci/pci.h>
}
#include <sys/sysinfo.h>
#include <QDir>

#include "OSLinux.h"

namespace PWTD::LNX {
    bool OSLinux::setupOSAccess() const {
        return true;
    }

    void OSLinux::unsetOSAccess() const {}

    QSet<PWTS::Feature> OSLinux::getCPUFeatures(const int numLogicalCPUs, const PWTS::CPUVendor vendor) const {
        QSet<PWTS::Feature> features;

        if (numLogicalCPUs > 1) {
            features.unite({PWTS::Feature::CPU_PARK_SYSFS, PWTS::Feature::SYSFS_GROUP});

            if (hasSMT())
                features.insert(PWTS::Feature::CPU_SMT_SYSFS);
        }

        if (QDir(QString("%1cpufreq").arg(sysfsCPU)).exists())
            features.unite({PWTS::Feature::CPUFREQ_SYSFS, PWTS::Feature::SYSFS_GROUP});

        if (QFile::exists(QString("%1current_governor").arg(sysfsCPUIdle)))
            features.unite({PWTS::Feature::CPUIDLE_GOV_SYSFS, PWTS::Feature::SYSFS_GROUP});

#ifdef WITH_AMD
        if (vendor == PWTS::CPUVendor::AMD)
            features.unite(getAMDCPUFeatures());
#endif

        return features;
    }

    QSet<PWTS::Feature> OSLinux::getIntelGPUFeatures(const int index) const {
        QSet<PWTS::Feature> features;

        if (hasIntelGPURPSFreq(index)) {
            features.unite({PWTS::Feature::INTEL_GPU_RPS_FREQ_SYSFS, PWTS::Feature::INTEL_GPU_SYSFS_GROUP});

            if (hasIntelGPUBoost(index))
                features.insert(PWTS::Feature::INTEL_GPU_BOOST_SYSFS);
        }

        return features;
    }

    QSet<PWTS::Feature> OSLinux::getAMDGPUFeatures(const int index) const {
        QSet<PWTS::Feature> features;

        if (hasAMDGPUDpmForcePerfLevel(index))
            features.unite({PWTS::Feature::AMD_GPU_DPM_FORCE_PERF_LEVEL_SYSFS, PWTS::Feature::AMD_GPU_SYSFS_GROUP});
        else if (hasAMDGPUPowerDpmState(index))
            features.unite({PWTS::Feature::AMD_GPU_POWER_DPM_STATE_SYSFS, PWTS::Feature::AMD_GPU_SYSFS_GROUP});

        return features;
    }

    std::pair<PWTS::GPUVendor, QSet<PWTS::Feature>> OSLinux::getGPUFeatures(const int index, const PWTS::GPUVendor vendor) const {
        switch (vendor) {
            case PWTS::GPUVendor::Intel:
                return {PWTS::GPUVendor::Intel, getIntelGPUFeatures(index)};
            case PWTS::GPUVendor::AMD:
                return {PWTS::GPUVendor::AMD, getAMDGPUFeatures(index)};
            default:
                break;
        }

        return {};
    }

    bool OSLinux::hasFanControls() const {
#ifdef WITH_GPD_FAN
        if (QDir(sysfsGpdfan).exists())
            return true;
#endif

        return false;
    }

    void OSLinux::fillIntelGPUData(const int index, const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::INTEL_GPU_SYSFS_GROUP))
            return;

        PWTS::LNX::LinuxIntelGPUData gdata;

        if (features.contains(PWTS::Feature::INTEL_GPU_RPS_FREQ_SYSFS)) {
            gdata.rpsLimits = getIntelGPURPSLimits(index);
            gdata.frequency = getIntelGPUFrequency(index);
        }

        if (features.contains(PWTS::Feature::INTEL_GPU_BOOST_SYSFS))
            gdata.boostFrequency = getIntelGPUBoost(index);

        packet.linuxData->intelGpuData.insert(index, gdata);
    }

    void OSLinux::fillAMDGPUData(const int index, const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::AMD_GPU_SYSFS_GROUP))
            return;

        PWTS::LNX::LinuxAMDGPUData gdata;

        if (features.contains(PWTS::Feature::AMD_GPU_DPM_FORCE_PERF_LEVEL_SYSFS)) {
            gdata.odRanges = getAMDGPUODRanges(index);
            gdata.dpmForcePerfLevel = getAMDGPUDpmForcePerfLevel(index);
        }

        if (features.contains(PWTS::Feature::AMD_GPU_POWER_DPM_STATE_SYSFS))
            gdata.powerDpmState = getAMDGPUPowerDpmState(index);

        packet.linuxData->amdGpuData.insert(index, gdata);
    }

    void OSLinux::fillPackageData(const PWTS::Features &features, const PWTS::DaemonPacket &packet) const {
        packet.linuxData->blockDevicesQueSched = getBlockDevices();
        packet.linuxData->miscPMDevices = getMiscPMDevices();

        if (features.cpu.contains(PWTS::Feature::SYSFS_GROUP)) {
            if (features.cpu.contains(PWTS::Feature::CPU_SMT_SYSFS))
                packet.linuxData->smtState = getSMT();

            if (features.cpu.contains(PWTS::Feature::CPUIDLE_GOV_SYSFS)) {
                packet.linuxData->cpuIdleAvailableGovernors = getAvailableCPUIdleGovernors();
                packet.linuxData->cpuIdleGovernor = getCPUIdleGovernor();
            }
        }

        for (const auto &[gpuIdx, gpuFeatures]: features.gpus.asKeyValueRange()) {
            switch (gpuFeatures.first) {
                case PWTS::GPUVendor::Intel:
                    fillIntelGPUData(gpuIdx, gpuFeatures.second, packet);
                    break;
                case PWTS::GPUVendor::AMD:
                    fillAMDGPUData(gpuIdx, gpuFeatures.second, packet);
                    break;
                default:
                    break;
            }
        }
    }

    void OSLinux::fillThreadData(const int cpu, const PWTS::Features &features, const PWTS::DaemonPacket &packet) const {
        PWTS::LNX::LinuxThreadData thdData {};

        if (!features.cpu.contains(PWTS::Feature::SYSFS_GROUP)) {
            packet.linuxData->threadData.append(thdData);
            return;
        }

        if (features.cpu.contains(PWTS::Feature::CPU_PARK_SYSFS)) {
            thdData.cpuLogicalOffAvailable = hasCPULogicalOffFeature(cpu);
            thdData.coreID = getCoreID(cpu);
            thdData.cpuOnlineStatus = getCPUOnlineStatus(cpu);
        }

        if (features.cpu.contains(PWTS::Feature::CPUFREQ_SYSFS)) {
            thdData.cpuFrequencyLimits = getCPUFrequencyLimits(cpu);
            thdData.cpuFrequency = getCPUFrequency(cpu);
            thdData.scalingAvailableGovernors = getCPUScalingAvailableGovernors(cpu);
            thdData.scalingGovernor = getCPUScalingGovernor(cpu);
        }

        packet.linuxData->threadData.append(thdData);
    }

    void OSLinux::fillDaemonPacket(const PWTS::Features &features, const PWTS::CPUVendor cpuVendor, const int numLogicalCPUs, PWTS::DaemonPacket &packet) const {
        packet.linuxData = QSharedPointer<PWTS::LNX::LinuxData>::create();

        fillPackageData(features, packet);

        for (int i=0; i<numLogicalCPUs; ++i)
            fillThreadData(i, features, packet);

#ifdef WITH_AMD
        if (cpuVendor == PWTS::CPUVendor::AMD)
            fillAMDDaemonPacket(features.cpu, numLogicalCPUs, packet);
#endif
    }

    void OSLinux::applyIntelGPUSettings(const int index, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::INTEL_GPU_SYSFS_GROUP) || !packet.linuxData->intelGpuData.contains(index))
            return;

        if (features.contains(PWTS::Feature::INTEL_GPU_RPS_FREQ_SYSFS) && !setIntelGPUFrequency(index, packet.linuxData->intelGpuData[index].frequency))
            errors.insert(PWTS::DError::W_INTEL_GPU_FREQ);

        if (features.contains(PWTS::Feature::INTEL_GPU_BOOST_SYSFS) && !setIntelGPUBoost(index, packet.linuxData->intelGpuData[index].boostFrequency))
            errors.insert(PWTS::DError::W_INTEL_GPU_BOOST);
    }

    void OSLinux::applyAMDGPUSettings(const int index, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::AMD_GPU_SYSFS_GROUP) || !packet.linuxData->amdGpuData.contains(index))
            return;

        if (features.contains(PWTS::Feature::AMD_GPU_DPM_FORCE_PERF_LEVEL_SYSFS) && !setAMDGPUDpmForcePerfLevel(index, packet.linuxData->amdGpuData[index].dpmForcePerfLevel))
            errors.insert(PWTS::DError::W_AMD_GPU_DPM_FORCE_PERF_LEVEL);

        if (features.contains(PWTS::Feature::AMD_GPU_POWER_DPM_STATE_SYSFS) && !setAMDGPUPowerDpmState(index, packet.linuxData->amdGpuData[index].powerDpmState))
            errors.insert(PWTS::DError::W_AMD_GPU_POWER_DPM_STATE);
    }

    void OSLinux::applyPackageSettings(const PWTS::Features &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!setBlockDevices(packet.linuxData->blockDevicesQueSched))
            errors.insert(PWTS::DError::W_BLOCK_DEVICES);

        if (!setMiscPMDevices(packet.linuxData->miscPMDevices))
            errors.insert(PWTS::DError::W_MISC_PM_DEVICES);

        if (features.cpu.contains(PWTS::Feature::SYSFS_GROUP)) {
            if (features.cpu.contains(PWTS::Feature::CPU_SMT_SYSFS) && !setSMT(packet.linuxData->smtState))
                errors.insert(PWTS::DError::W_CPU_SMT);

            if (features.cpu.contains(PWTS::Feature::CPUIDLE_GOV_SYSFS) && !setCPUIdleGovernor(packet.linuxData->cpuIdleGovernor))
                errors.insert(PWTS::DError::W_CPU_IDLE_GOV);
        }

        for (const auto [gpuIndex, gpuFeatures]: features.gpus.asKeyValueRange()) {
            switch (gpuFeatures.first) {
                case PWTS::GPUVendor::Intel:
                    applyIntelGPUSettings(gpuIndex, gpuFeatures.second, packet, errors);
                    break;
                case PWTS::GPUVendor::AMD:
                    applyAMDGPUSettings(gpuIndex, gpuFeatures.second, packet, errors);
                    break;
                default:
                    break;
            }
        }
    }

    void OSLinux::applyThreadSettings(const int cpu, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::SYSFS_GROUP))
            return;

        const PWTS::LNX::LinuxThreadData &data = packet.linuxData->threadData[cpu];

        if (features.contains(PWTS::Feature::CPU_PARK_SYSFS) && !setCPUOnlineStatus(cpu, data.cpuOnlineStatus))
            errors.insert(PWTS::DError::W_CPUS_ONLINE);

        if (features.contains(PWTS::Feature::CPUFREQ_SYSFS)) {
            if (!setCPUFrequency(cpu, data.cpuFrequency))
                errors.insert(PWTS::DError::W_CPU_FREQ_MIN_MAX);

            if (!setCPUScalingGovernor(cpu, data.scalingGovernor))
                errors.insert(PWTS::DError::W_CPU_SCALING_GOV);
        }
    }

    QSet<PWTS::DError> OSLinux::applySettings(const PWTS::Features &features, const PWTS::CPUVendor cpuVendor, const int numLogicalCPUs, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const {
        const QSharedPointer<PWTS::LNX::LinuxData> ldata = packet.linuxData;
        QSet<PWTS::DError> errors;

        if (ldata.isNull()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write("empty data");

            return errors;

        } else if (numLogicalCPUs != ldata->threadData.size()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("thread count mismatch: %1 / %2").arg(numLogicalCPUs).arg(ldata->threadData.size()));

            errors.insert(PWTS::DError::THREAD_DATA_MISMATCH);
            return errors;
        }

        applyPackageSettings(features, packet, errors);

        for (int i=0; i<numLogicalCPUs; ++i)
            applyThreadSettings(i, features.cpu, packet, errors);

#ifdef WITH_AMD
        if (cpuVendor == PWTS::CPUVendor::AMD)
            applyAMDSettings(features.cpu, numLogicalCPUs, coreIdxList, packet, errors);
#endif

        return errors;
    }

    QString OSLinux::readSysfs(const QString &path, const bool existsErrorLog) const {
        QFile sysfsF {path};

        if (!sysfsF.exists()) {
            if (existsErrorLog && logger->isLevel(PWTS::LogLevel::Warning))
                logger->write(QString("'%1' does not exist").arg(path));

            return "";

        } else if (!sysfsF.open(ROTextOpenFlags)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to open '%1': %2").arg(path, sysfsF.errorString()));

            return "";
        }

        return QString::fromUtf8(sysfsF.readAll()).trimmed();
    }

    bool OSLinux::writeSysfs(const QString &path, const QString &value) const {
        QFile sysfsF {path};
        QTextStream ts {&sysfsF};
        bool success;

        if (!sysfsF.open(WOTextOpenFlags)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to open '%1': %2").arg(path, sysfsF.errorString()));

            return false;
        }

        ts << value;
        ts.flush();

        success = ts.status() == QTextStream::Ok;

        if (!success && logger->isLevel(PWTS::LogLevel::Error))
            logger->write(QString("failed to write '%1', status: %2").arg(path).arg(ts.status()));

        return success;
    }

    bool OSLinux::hasSMT() const {
        const QString smt = readSysfs(sysfsSMT);

        return !smt.isEmpty() && smt != "notsupported";
    }

    PWTS::ROData<bool> OSLinux::hasCPULogicalOffFeature(const int cpu) const {
        return PWTS::ROData<bool>(QFile::exists(QString("%1cpu%2/online").arg(sysfsCPU).arg(cpu)), true);
    }

    bool OSLinux::deviceHasRuntimePM(const QString &path) const {
        const QString runtimeSuspendedTime = readSysfs(QString("%1/power/runtime_suspended_time").arg(path), false);
        QString runtimeActiveTime;
        ulong runtimeV;
        bool res;

        if (runtimeSuspendedTime.isEmpty())
            return false;

        runtimeV = runtimeSuspendedTime.toULong(&res);
        if (res && runtimeV > 0)
            return true;

        runtimeActiveTime = readSysfs(QString("%1/power/runtime_active_time").arg(path), false);
        if (runtimeActiveTime.isEmpty())
            return false;

        runtimeV = runtimeActiveTime.toULong(&res);

        return (res && runtimeV > 0);
    }

    int OSLinux::getOnlineCPUCount(const int numLogicalCPUs) const {
        return get_nprocs();
    }

    QString OSLinux::getBiosVendor() const {
        return readSysfs(QString("%1bios_vendor").arg(sysfsDMI));
    }

    QString OSLinux::getBiosVersion() const {
        return readSysfs(QString("%1bios_version").arg(sysfsDMI));
    }

    QString OSLinux::getBiosDate() const {
        return readSysfs(QString("%1bios_date").arg(sysfsDMI));
    }

    QString OSLinux::getECVersion() const {
        return readSysfs(QString("%1ec_firmware_release").arg(sysfsDMI));
    }

    QString OSLinux::getProductName() const {
        return readSysfs(QString("%1product_name").arg(sysfsDMI));
    }

    QString OSLinux::getManufacturer() const {
        QString vendorDmi = readSysfs(QString("%1sys_vendor").arg(sysfsDMI), false);

        if (vendorDmi.isEmpty())
            return readSysfs(QString("%1board_vendor").arg(sysfsDMI));

        return vendorDmi;
    }

    QString OSLinux::getMicrocodeRevision(const int cpu) const {
        return readSysfs(QString("%1cpu%2/microcode/version").arg(sysfsCPU).arg(cpu));
    }

    quint64 OSLinux::getAvailableRam() const {
        struct sysinfo sinfo {};

        if (sysinfo(&sinfo) != 0)
            return 0;

        return (static_cast<quint64>(sinfo.totalram) * sinfo.mem_unit);
    }

    quint64 OSLinux::getSwapSize() const {
        struct sysinfo sinfo {};

        if (sysinfo(&sinfo) != 0)
            return 0;

        return (static_cast<quint64>(sinfo.totalswap) * sinfo.mem_unit);
    }

    QList<int> OSLinux::getGPUIndexList() const {
        const QDirListing drmIt(sysfsDRM, {"card*"}, QDirListing::IteratorFlag::DirsOnly | QDirListing::IteratorFlag::ResolveSymlinks);
        QList<int> gpus;

        for (const QDirListing::DirEntry &entry: drmIt) {
            const QRegularExpressionMatch match = gpuCardFolderRex.match(entry.baseName());
            int index;
            bool res;

            if (!match.hasMatch())
                continue;

            index = match.captured(1).toInt(&res);
            if (res)
                gpus.append(index);
        }

        return gpus;
    }

    PWTS::GPUVendor OSLinux::getGPUVendor(const int index) const {
        const QString vendorID = readSysfs(QString("%1card%2/device/vendor").arg(sysfsDRM).arg(index)).toLower();

        if (vendorID.isEmpty())
            return PWTS::GPUVendor::Unknown;
        else if (vendorID == "0x1002")
            return PWTS::GPUVendor::AMD;
        else if (vendorID == "0x8086")
            return PWTS::GPUVendor::Intel;
        else if (vendorID == "0x10de")
            return PWTS::GPUVendor::NVIDIA;

        return PWTS::GPUVendor::Unknown;
    }

    QString OSLinux::getGPUDeviceID(const int index) const {
        return readSysfs(QString("%1card%2/device/device").arg(sysfsDRM).arg(index));
    }

    QString OSLinux::getGPURevisionID(const int index) const {
        return readSysfs(QString("%1card%2/device/revision").arg(sysfsDRM).arg(index));
    }

    QString OSLinux::getGPUVBiosVersion(const int index) const {
        return readSysfs(QString("%1card%2/device/vbios_version").arg(sysfsDRM).arg(index));
    }

    QString OSLinux::getFanControlPath(const FanBoard board) const {
        switch (board) {
#ifdef WITH_GPD_FAN
            case FanBoard::GPD_WIN4:
            case FanBoard::GPD_WIN_MINI:
            case FanBoard::GPD_WIN_MAX2:
            case FanBoard::GPD_DUO:
            case FanBoard::GPD_MPC2:
                return getGPDFanHWMon();
#endif
            default:
                break;
        }

        return "";
    }

    PWTS::RWData<int> OSLinux::getFanMode(const FanControls &controls) const {
        switch (controls.board) {
#ifdef WITH_GPD_FAN
            case FanBoard::GPD_WIN4:
            case FanBoard::GPD_WIN_MINI:
            case FanBoard::GPD_WIN_MAX2:
            case FanBoard::GPD_DUO:
            case FanBoard::GPD_MPC2:
                return getGPDFanMode(controls.controlPath);
#endif
            default:
                break;
        }

        return {};
    }

    PWTS::ROData<int> OSLinux::getFanSpeed(const FanControls &controls) const {
        switch (controls.board) {
#ifdef WITH_GPD_FAN
            case FanBoard::GPD_WIN4:
            case FanBoard::GPD_WIN_MINI:
            case FanBoard::GPD_WIN_MAX2:
            case FanBoard::GPD_DUO:
            case FanBoard::GPD_MPC2:
                return getGPDFanSpeed(controls.controlPath);
#endif
            default:
                break;
        }

        return {};
    }

    bool OSLinux::setFanMode(const FanControls &controls, const PWTS::RWData<int> &mode) const {
        switch (controls.board) {
#ifdef WITH_GPD_FAN
            case FanBoard::GPD_WIN4:
            case FanBoard::GPD_WIN_MINI:
            case FanBoard::GPD_WIN_MAX2:
            case FanBoard::GPD_DUO:
            case FanBoard::GPD_MPC2:
                return setGPDFanMode(mode, controls.controlPath);
#endif
            default:
                break;
        }

        return false;
    }

    bool OSLinux::setFanSpeed(const FanControls &controls, const int speed) const {
        switch (controls.board) {
#ifdef WITH_GPD_FAN
            case FanBoard::GPD_WIN4:
            case FanBoard::GPD_WIN_MINI:
            case FanBoard::GPD_WIN_MAX2:
            case FanBoard::GPD_DUO:
            case FanBoard::GPD_MPC2:
                return setGPDFanSpeed(speed, controls.controlPath);
#endif
            default:
                break;
        }

        return false;
    }

    PWTS::ROData<int> OSLinux::getCoreID(const int cpu) const {
        const QString coreID = readSysfs(QString("%1cpu%2/topology/core_id").arg(sysfsCPU).arg(cpu));
        bool res;
        int id;

        if (coreID.isEmpty())
            return {};

        id = coreID.toInt(&res);

        return PWTS::ROData<int>(id, res);
    }

    QList<int> OSLinux::getCPUCoreIndexList() const {
        const QDirListing dit(sysfsCPU, {"cpu*"}, QDirListing::IteratorFlag::DirsOnly | QDirListing::IteratorFlag::ResolveSymlinks);
        const QRegularExpression cpuRex {R"(cpu([0-9]+)$)"};
        QMap<int, int> coreMap;

        for (const QDirListing::DirEntry &entry: dit) {
            const QRegularExpressionMatch match = cpuRex.match(entry.fileName());

            if (!match.hasMatch())
                continue;

            const QString id = readSysfs(QString("%1/topology/core_id").arg(entry.filePath()));
            const QString cpu = match.captured(1);
            bool idRes, cpuRes;
            const int idI = id.toInt(&idRes);
            const int cpuI = cpu.toInt(&cpuRes);

            if (logger->isLevel(PWTS::LogLevel::Info))
                logger->write(QString("found: cpu %1, core id %2").arg(cpu, id));

            if (!idRes || !cpuRes) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(QStringLiteral("failed to parse core id or cpu number"));

                return {};
            }

            if (!coreMap.contains(idI)) {
                if (logger->isLevel(PWTS::LogLevel::Info))
                    logger->write(QString("added core %1").arg(id));

                coreMap.insert(idI, cpuI);
            }
        }

        return coreMap.values();
    }

    PWTS::ROData<PWTS::LNX::CPUFrequencyLimits> OSLinux::getCPUFrequencyLimits(const int cpu) const {
        const QString cpufreqPath = QString("%1cpu%2/cpufreq").arg(sysfsCPU).arg(cpu);
        const QString minFreq = readSysfs(QString("%1/cpuinfo_min_freq").arg(cpufreqPath));
        const QString maxFreq = readSysfs(QString("%1/cpuinfo_max_freq").arg(cpufreqPath));
        const QString relatedCpus = readSysfs(QString("%1/related_cpus").arg(cpufreqPath));
        PWTS::LNX::CPUFrequencyLimits data;
        bool minRes, maxRes;

        if (minFreq.isEmpty() || maxFreq.isEmpty())
            return {};

        if (!relatedCpus.isEmpty()) {
            data.relatedCPUs = relatedCpus.split(u' ', Qt::SkipEmptyParts);
            data.relatedCPUs.removeOne(QString::number(cpu));
        }

        data.limit.max = maxFreq.toInt(&minRes) / 1000;
        data.limit.min = minFreq.toInt(&maxRes) / 1000;

        return PWTS::ROData<PWTS::LNX::CPUFrequencyLimits>(data, minRes && maxRes && data.limit.max > 0);
    }

    PWTS::RWData<PWTS::MinMax> OSLinux::getCPUFrequency(const int cpu) const {
        const QString cpufreqPath = QString("%1cpu%2/cpufreq").arg(sysfsCPU).arg(cpu);
        const QString minFreq = readSysfs(QString("%1/scaling_min_freq").arg(cpufreqPath));
        const QString maxFreq = readSysfs(QString("%1/scaling_max_freq").arg(cpufreqPath));
        bool minRes, maxRes;
        int min, max;

        if (minFreq.isEmpty() || maxFreq.isEmpty())
            return {};

        min = minFreq.toInt(&minRes) / 1000;
        max = maxFreq.toInt(&maxRes) / 1000;

        return PWTS::RWData<PWTS::MinMax>({
            .min = min,
            .max = max
        }, minRes && maxRes);
    }

    PWTS::RWData<QString> OSLinux::getSMT() const {
        const QString smt = readSysfs(sysfsSMT);

        return PWTS::RWData<QString>(smt, !smt.isEmpty());
    }

    PWTS::RWData<int> OSLinux::getCPUOnlineStatus(const int cpu) const {
        QFile online {QString("%1cpu%2/online").arg(sysfsCPU).arg(cpu)};
        const bool isValid = !online.exists() || online.open(ROTextOpenFlags);
        const int isOn = !online.exists() || (online.isOpen() && QString::fromUtf8(online.readAll()).trimmed() == "1");

        return PWTS::RWData<int>(isOn, isValid);
    }

    PWTS::ROData<PWTS::LNX::CPUScalingAvailableGovernors> OSLinux::getCPUScalingAvailableGovernors(const int cpu) const {
        const QString cpufreqPath = QString("%1cpu%2/cpufreq").arg(sysfsCPU).arg(cpu);
        const QString availGovernors = readSysfs(QString("%1/scaling_available_governors").arg(cpufreqPath));
        const QString relatedCpus = readSysfs(QString("%1/related_cpus").arg(cpufreqPath));
        PWTS::LNX::CPUScalingAvailableGovernors data;

        if (availGovernors.isEmpty() || relatedCpus.isEmpty())
            return {};

        data.availableGovernors = availGovernors.split(u' ', Qt::SkipEmptyParts);
        data.relatedCPUs = relatedCpus.split(u' ', Qt::SkipEmptyParts);

        data.relatedCPUs.removeOne(QString::number(cpu));

        return PWTS::ROData<PWTS::LNX::CPUScalingAvailableGovernors>(data, true);
    }

    PWTS::RWData<QString> OSLinux::getCPUScalingGovernor(const int cpu) const {
        const QString scalingGov = readSysfs(QString("%1cpu%2/cpufreq/scaling_governor").arg(sysfsCPU).arg(cpu));

        return PWTS::RWData<QString>(scalingGov, !scalingGov.isEmpty());
    }

    PWTS::ROData<QList<QString>> OSLinux::getAvailableCPUIdleGovernors() const {
        const QString availIdleGov = readSysfs(QString("%1available_governors").arg(sysfsCPUIdle));

        if (availIdleGov.isEmpty())
            return {};

        const QList<QString> availableGovernors = availIdleGov.split(u' ', Qt::SkipEmptyParts);

        return PWTS::ROData<QList<QString>>(availableGovernors, true);
    }

    PWTS::RWData<QString> OSLinux::getCPUIdleGovernor() const {
        const QString idleGov = readSysfs(QString("%1current_governor").arg(sysfsCPUIdle));

        return PWTS::RWData<QString>(idleGov, !idleGov.isEmpty());
    }

    QMap<QString, PWTS::LNX::BlockDeviceQueSched> OSLinux::getBlockDevices() const {
        const QDirListing dit(sysfsBlock, QDirListing::IteratorFlag::DirsOnly | QDirListing::IteratorFlag::ResolveSymlinks);
        QMap<QString, PWTS::LNX::BlockDeviceQueSched> blockDevs;

        for (const QDirListing::DirEntry &entry: dit) {
            const QString devPath = entry.filePath();
            const QString schedPath = QString("%1/queue/scheduler").arg(devPath);

            if (!QFile::exists(schedPath))
                continue;

            const QString model = readSysfs(QString("%1/device/model").arg(devPath), false);
            const QString vendor = readSysfs(QString("%1/device/vendor").arg(devPath), false);
            const QString device = devPath.split('/', Qt::SkipEmptyParts).last().trimmed();
            const QString scheduler = readSysfs(schedPath);
            PWTS::LNX::BlockDeviceQueSched blkDev;

            blkDev.name = QString("%1 %2").arg(vendor, model).trimmed();

            for (const QString &sched: scheduler.split(' ', Qt::SkipEmptyParts)) {
                const bool isSelected = sched.startsWith('[');

                blkDev.availableQueueSchedulers.append(isSelected ? sched.mid(1, sched.size() - 2) : sched);

                if (isSelected)
                    blkDev.scheduler = blkDev.availableQueueSchedulers.last();
            }

            if (device.isEmpty() || blkDev.availableQueueSchedulers.isEmpty()) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(QString("invalid block device %1, or no schedulers available").arg(device));

                continue;
            }

            blockDevs.insert(device, blkDev);
        }

        return blockDevs;
    }

    QList<PWTS::LNX::MiscPMDevice> OSLinux::getMiscPMI2cDevices() const {
        const QDirListing dit("/sys/bus/i2c/devices", QDirListing::IteratorFlag::DirsOnly | QDirListing::IteratorFlag::ResolveSymlinks);
        QList<PWTS::LNX::MiscPMDevice> i2cDevs;

        for (const QDirListing::DirEntry &entry: dit) {
            const QString devPath = entry.filePath();
            const bool isAdapter = QFile::exists(QString("%1/new_device").arg(devPath));
            const QString controlPath = QString("%1/%2power/control").arg(devPath, isAdapter ? "device/":"");
            const QString name = readSysfs(QString("%1/name").arg(devPath), false);
            const QString control = readSysfs(controlPath, false);

            if (name.isEmpty() || control.isEmpty() || !deviceHasRuntimePM(QString("%1/device").arg(devPath)))
                continue;

            i2cDevs.append({
                .name = QString("%1 [I2C %2]").arg(name, isAdapter ? "Adapter":"Device"),
                .control = controlPath,
                .controlValue = control
            });
        }

        return i2cDevs;
    }

    QList<PWTS::LNX::MiscPMDevice> OSLinux::getMiscPMPCIDevices() const {
        const QDirListing dit("/sys/bus/pci/devices", QDirListing::IteratorFlag::DirsOnly | QDirListing::IteratorFlag::ResolveSymlinks);
        QList<PWTS::LNX::MiscPMDevice> pciDevs;
        struct pci_access *pacc = pci_alloc();

        if (pacc != nullptr)
            pci_init(pacc);

        for (const QDirListing::DirEntry &entry: dit) {
            const QString devPath = entry.filePath();
            const QString controlPath = QString("%1/power/control").arg(devPath);
            const QString control = readSysfs(controlPath, false);
            QString device;
            QString vendor;
            QString devName;

            if (control.isEmpty() || !deviceHasRuntimePM(devPath))
                continue;

            device = readSysfs(QString("%1/device").arg(devPath), false);
            vendor = readSysfs(QString("%1/vendor").arg(devPath), false);

            if (pacc != nullptr && !device.isEmpty() && !vendor.isEmpty()) {
                char nameBuf[1024] = {};
                uint16_t deviceID, vendorID;
                bool dres, vres;

                deviceID = device.toUInt(&dres, 16);
                vendorID = vendor.toUInt(&vres, 16);

                if (dres && vres)
                    pci_lookup_name(pacc, nameBuf, sizeof(nameBuf), PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE, vendorID, deviceID);

                devName.assign(nameBuf);
            }

            if (devName.isEmpty())
                devName = QFileInfo(controlPath).baseName();

            pciDevs.append({
               .name = devName,
               .control = controlPath,
               .controlValue = control
            });

            for (const QDirListing::DirEntry &ataEntry: QDirListing(devPath, {"ata*"}, QDirListing::IteratorFlag::DirsOnly | QDirListing::IteratorFlag::ResolveSymlinks)) {
                const QString ataDev = ataEntry.filePath();
                const QString ataControlPath = QString("%1/power/control").arg(ataDev);
                const QString ataControl = readSysfs(ataControlPath, false);

                if (ataControl.isEmpty() || !deviceHasRuntimePM(ataDev))
                    continue;

                pciDevs.append({
                    .name = QString("%1 [%2 port]").arg(devName, ataEntry.baseName()),
                    .control = ataControlPath,
                    .controlValue = ataControl
                });
            }
        }

        if (pacc != nullptr)
            pci_cleanup(pacc);

        return pciDevs;
    }

    QList<PWTS::LNX::MiscPMDevice> OSLinux::getMiscPMBlockDevices() const {
        const QDirListing dit("/sys/block", {"sd*"}, QDirListing::IteratorFlag::DirsOnly | QDirListing::IteratorFlag::ResolveSymlinks);
        QList<PWTS::LNX::MiscPMDevice> blkDevs;

        for (const QDirListing::DirEntry &entry: dit) {
            const QString blkPath = entry.filePath();
            const QString controlPath = QString("%1/device/power/control").arg(blkPath);
            const QString control = readSysfs(controlPath, false);

            if (control.isEmpty() || !deviceHasRuntimePM(QString("%1/device").arg(blkPath)))
                continue;

            blkDevs.append({
                .name = QString("Disk [%1]").arg(entry.baseName()),
                .control = controlPath,
                .controlValue = control
            });
        }

        return blkDevs;
    }

    QList<PWTS::LNX::MiscPMDevice> OSLinux::getMiscPMUSBDevices() const {
        const QDirListing dit("/sys/bus/usb/devices", QDirListing::IteratorFlag::DirsOnly | QDirListing::IteratorFlag::ResolveSymlinks);
        QList<PWTS::LNX::MiscPMDevice> usbDevs;

        for (const QDirListing::DirEntry &entry: dit) {
            if (!QFile::exists(QString("%1/power/active_duration").arg(entry.filePath())))
                continue;

            const QString devPath = entry.filePath();
            const QString controlPath = QString("%1/power/control").arg(devPath);
            const QString control = readSysfs(controlPath, false);
            bool hasUSBPM = true;
            QString idProduct;
            QString idVendor;
            QString devName;

            if (control.isEmpty())
                continue;

            for (const QDirListing::DirEntry &usbEntry: QDirListing(devPath, QDirListing::IteratorFlag::DirsOnly)) {
                if (!usbEntry.baseName().at(0).isDigit())
                    continue;

                const QString autosuspend = readSysfs(QString("%1/supports_autosuspend").arg(usbEntry.filePath()), false);

                if (autosuspend != "0")
                    continue;

                hasUSBPM = false;
                break;
            }

            if (!hasUSBPM)
                continue;

            idProduct = readSysfs(QString("%1/idProduct").arg(devPath), false);
            idVendor = readSysfs(QString("%1/idVendor").arg(devPath), false);

            if (idProduct.isEmpty() || idVendor.isEmpty())
                continue;

            devName = readSysfs(QString("%1/product").arg(devPath), false);

            usbDevs.append({
                .name = QString("%1 [%2:%3]").arg(devName.isEmpty() ? "Unknown USB device" : devName, idVendor, idProduct),
                .control = controlPath,
                .controlValue = control
            });
        }

        return usbDevs;
    }

    QList<PWTS::LNX::MiscPMDevice> OSLinux::getMiscPMDevices() const {
        QList<PWTS::LNX::MiscPMDevice> ret;

        ret.append(getMiscPMI2cDevices());
        ret.append(getMiscPMPCIDevices());
        ret.append(getMiscPMBlockDevices());
        ret.append(getMiscPMUSBDevices());

        return ret;
    }

    bool OSLinux::setCPUFrequency(const int cpu, const PWTS::RWData<PWTS::MinMax> &data) const {
        if (!data.isValid())
            return true;

        const PWTS::MinMax freq = data.getValue();
        const bool minWriteRes = writeSysfs(QString("%1cpu%2/cpufreq/scaling_min_freq").arg(sysfsCPU).arg(cpu), QString::number(freq.min * 1000));
        const bool maxWriteRes = writeSysfs(QString("%1cpu%2/cpufreq/scaling_max_freq").arg(sysfsCPU).arg(cpu), QString::number(freq.max * 1000));

        return minWriteRes && maxWriteRes;
    }

    bool OSLinux::setSMT(const PWTS::RWData<QString> &state) const {
        if (!state.isValid())
            return true;

        return writeSysfs(sysfsSMT, state.getValue());
    }

    bool OSLinux::setCPUOnlineStatus(const int cpu, const PWTS::RWData<int> &data) const {
        const QString onlinePath = QString("%1cpu%2/online").arg(sysfsCPU).arg(cpu);

        if (!QFile::exists(onlinePath) || !data.isValid())
            return true;

        return writeSysfs(onlinePath, QString::number(data.getValue()));
    }

    bool OSLinux::setCPUScalingGovernor(const int cpu, const PWTS::RWData<QString> &data) const {
        if (!data.isValid())
            return true;

        return writeSysfs(QString("%1cpu%2/cpufreq/scaling_governor").arg(sysfsCPU).arg(cpu), data.getValue());
    }

    bool OSLinux::setCPUIdleGovernor(const PWTS::RWData<QString> &data) const {
        if (!data.isValid())
            return true;

        return writeSysfs(QString("%1current_governor").arg(sysfsCPUIdle), data.getValue());
    }

    bool OSLinux::setBlockDevices(const QMap<QString, PWTS::LNX::BlockDeviceQueSched> &blockDevList) const {
        bool success = true;

        for (const auto &[dev, data]: blockDevList.asKeyValueRange()) {
            const QString schedPath = QString("%1/%2/queue/scheduler").arg(sysfsBlock, dev);

            if (!QFile::exists(schedPath)) // probably detached device, no error
                continue;

            if (!writeSysfs(schedPath, data.scheduler))
                success = false;
        }

        return success;
    }

    bool OSLinux::setMiscPMDevices(const QList<PWTS::LNX::MiscPMDevice> &miscPMDevList) const {
        bool success = true;

        for (const PWTS::LNX::MiscPMDevice &miscPMDev: miscPMDevList) {
            if (!miscPmDevPathRex.match(miscPMDev.control).hasMatch() && !miscPmBlockPathRex.match(miscPMDev.control).hasMatch()) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(QString("invalid control path: %1").arg(miscPMDev.control));

                continue;
            }

            if (!QFile::exists(miscPMDev.control)) // probably detached device, no error
                continue;

            if (!writeSysfs(miscPMDev.control, miscPMDev.controlValue))
                success = false;
        }

        return success;
    }

#ifdef WITH_AMD
    QSet<PWTS::Feature> OSLinux::getAMDCPUFeatures() const {
        QSet<PWTS::Feature> features;

        if (QFile::exists(QString("%1/amd_pstate").arg(sysfsCPU)))
            features.unite({PWTS::Feature::AMD_PSTATE_SYSFS, PWTS::Feature::SYSFS_GROUP});

        return features;
    }

    void OSLinux::fillAMDPackageData(const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::SYSFS_GROUP))
            return;

        if (features.contains(PWTS::Feature::AMD_PSTATE_SYSFS))
            packet.linuxAmdData->pstateStatus = getAMDPStateStatus();
    }

    void OSLinux::fillAMDThreadData(const int cpu, const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const {
        PWTS::LNX::AMD::LinuxAMDThreadData thdData {};

        if (!features.contains(PWTS::Feature::SYSFS_GROUP)) {
            packet.linuxAmdData->threadData.append(thdData);
            return;
        }

        if (features.contains(PWTS::Feature::AMD_PSTATE_SYSFS)) {
            thdData.pstateData = getAMDPStateData(cpu);
            thdData.epp = getAMDPStateEPPPreference(cpu);
        }

        packet.linuxAmdData->threadData.append(thdData);
    }

    void OSLinux::fillAMDDaemonPacket(const QSet<PWTS::Feature> &features, const int numLogicalCPUs, PWTS::DaemonPacket &packet) const {
        packet.linuxAmdData = QSharedPointer<PWTS::LNX::AMD::LinuxAMDData>::create();

        fillAMDPackageData(features, packet);

        for (int i=0; i<numLogicalCPUs; ++i)
            fillAMDThreadData(i, features, packet);
    }

    void OSLinux::applyAMDPackageSettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::SYSFS_GROUP))
            return;

        const QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> data = packet.linuxAmdData;

        if (features.contains(PWTS::Feature::AMD_PSTATE_SYSFS) && !setAMDPStateStatus(data->pstateStatus))
            errors.insert(PWTS::DError::W_AMDPSTATE_SYSFS);
    }

    void OSLinux::applyAMDThreadSettings(const int cpu, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        if (!features.contains(PWTS::Feature::SYSFS_GROUP))
            return;

        const PWTS::LNX::AMD::LinuxAMDThreadData &data = packet.linuxAmdData->threadData[cpu];

        if (features.contains(PWTS::Feature::AMD_PSTATE_SYSFS) && !setAMDPStateEPPPreference(cpu, data.epp))
            errors.insert(PWTS::DError::W_AMDPSTATE_EPP_SYSFS);
    }

    void OSLinux::applyAMDSettings(const QSet<PWTS::Feature> &features, const int numLogicalCPUs, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        const QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> ldata = packet.linuxAmdData;

        if (ldata.isNull()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write("empty data");

            return;

        } else if (numLogicalCPUs != ldata->threadData.size()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("thread count mismatch: %1 / %2").arg(numLogicalCPUs).arg(ldata->threadData.size()));

            errors.insert(PWTS::DError::THREAD_DATA_MISMATCH);
            return;
        }

        applyAMDPackageSettings(features, packet, errors);

        for (int i=0; i<numLogicalCPUs; ++i)
            applyAMDThreadSettings(i, features, packet, errors);
    }

     PWTS::ROData<PWTS::LNX::AMD::AMDPStateData> OSLinux::getAMDPStateData(const int cpu) const { //todo partial impl
        const QString eppPrefs = readSysfs(QString("%1cpu%2/cpufreq/energy_performance_available_preferences").arg(sysfsCPU).arg(cpu));
        PWTS::LNX::AMD::AMDPStateData data;

         if (eppPrefs.isEmpty())
             return {};

        data.eppAvailablePrefs = eppPrefs.split(u' ', Qt::SkipEmptyParts);

        return PWTS::ROData<PWTS::LNX::AMD::AMDPStateData>(data, true);
    }

    PWTS::RWData<QString> OSLinux::getAMDPStateStatus() const {
        const QString status = readSysfs(QString("%1amd_pstate/status").arg(sysfsCPU));

        return PWTS::RWData<QString>(status, !status.isEmpty());
    }

    PWTS::RWData<QString> OSLinux::getAMDPStateEPPPreference(const int cpu) const {
        const QString epp = readSysfs(QString("%1cpu%2/cpufreq/energy_performance_preference").arg(sysfsCPU).arg(cpu));

        return PWTS::RWData<QString>(epp, !epp.isEmpty());
    }

    bool OSLinux::setAMDPStateStatus(const PWTS::RWData<QString> &data) const {
        if (!data.isValid())
            return true;

        return writeSysfs(QString("%1amd_pstate/status").arg(sysfsCPU), data.getValue());
    }

    bool OSLinux::setAMDPStateEPPPreference(const int cpu, const PWTS::RWData<QString> &data) const {
        const QString eppPath = QString("%1cpu%2/cpufreq/energy_performance_preference").arg(sysfsCPU).arg(cpu);

        if (!QFile::exists(eppPath) || !data.isValid())
            return true;

        return writeSysfs(eppPath, data.getValue());
    }
#endif

    bool OSLinux::hasIntelGPURPSFreq(const int index) const {
        return QFile::exists(QString("%1card%2/gt_RP0_freq_mhz").arg(sysfsDRM).arg(index)) &&
                QFile::exists(QString("%1card%2/gt_RPn_freq_mhz").arg(sysfsDRM).arg(index));
    }

    bool OSLinux::hasIntelGPUBoost(const int index) const {
        return QFile::exists(QString("%1card%2/gt_boost_freq_mhz").arg(sysfsDRM).arg(index));
    }

    PWTS::ROData<PWTS::LNX::Intel::GPURPSLimits> OSLinux::getIntelGPURPSLimits(const int index) const {
        const QString rp0 = readSysfs(QString("%1card%2/gt_RP0_freq_mhz").arg(sysfsDRM).arg(index));
        const QString rpn = readSysfs(QString("%1card%2/gt_RPn_freq_mhz").arg(sysfsDRM).arg(index));
        bool res0, resN;
        int rp0I, rpnI;

        if (rp0.isEmpty() || rpn.isEmpty())
            return {};

        rp0I = rp0.toInt(&res0);
        rpnI = rpn.toInt(&resN);

        return PWTS::ROData<PWTS::LNX::Intel::GPURPSLimits>({
            .rp0 = rp0I,
            .rpn = rpnI
        }, res0 && resN);
    }

    PWTS::RWData<PWTS::MinMax> OSLinux::getIntelGPUFrequency(const int index) const {
        const QString min = readSysfs(QString("%1card%2/gt_min_freq_mhz").arg(sysfsDRM).arg(index));
        const QString max = readSysfs(QString("%1card%2/gt_max_freq_mhz").arg(sysfsDRM).arg(index));
        bool resMin, resMax;
        int minI, maxI;

        if (min.isEmpty() || max.isEmpty())
            return {};

        minI = min.toInt(&resMin);
        maxI = max.toInt(&resMax);

        return PWTS::RWData<PWTS::MinMax>({
            .min = minI,
            .max = maxI
        }, resMin && resMax);
    }

    bool OSLinux::setIntelGPUFrequency(const int index, const PWTS::RWData<PWTS::MinMax> &data) const {
        if (!data.isValid())
            return true;

        const PWTS::MinMax freq = data.getValue();
        const QString minPath = QString("%1card%2/gt_min_freq_mhz").arg(sysfsDRM).arg(index);
        const QString maxPath = QString("%1card%2/gt_max_freq_mhz").arg(sysfsDRM).arg(index);

        return writeSysfs(minPath, QString::number(freq.min)) && writeSysfs(maxPath, QString::number(freq.max));
    }

    PWTS::RWData<int> OSLinux::getIntelGPUBoost(const int index) const {
        const QString boost = readSysfs(QString("%1card%2/gt_boost_freq_mhz").arg(sysfsDRM).arg(index));
        int boostI;
        bool res;

        if (boost.isEmpty())
            return {};

        boostI = boost.toInt(&res);

        return PWTS::RWData<int>(boostI, res);
    }

    bool OSLinux::setIntelGPUBoost(const int index, const PWTS::RWData<int> &data) const {
        if (!data.isValid())
            return true;

        return writeSysfs(QString("%1card%2/gt_boost_freq_mhz").arg(sysfsDRM).arg(index), QString::number(data.getValue()));
    }

    bool OSLinux::hasAMDGPUDpmForcePerfLevel(const int index) const {
        return QFile::exists(QString("%1card%2/device/power_dpm_force_performance_level").arg(sysfsDRM).arg(index));
    }

    bool OSLinux::hasAMDGPUPowerDpmState(int index) const {
        return QFile::exists(QString("%1card%2/device/power_dpm_state").arg(sysfsDRM).arg(index));
    }

    PWTS::ROData<PWTS::LNX::AMD::GPUODRanges> OSLinux::getAMDGPUODRanges(const int index) const {
        QString odclk = readSysfs(QString("%1card%2/device/pp_od_clk_voltage").arg(sysfsDRM).arg(index));
        QTextStream ts {&odclk};
        bool sclkValid = false;
        int sclkMin = 0, sclkMax = 0;
        QString tsLine;

        if (odclk.isEmpty())
            return {};

        while (ts.readLineInto(&tsLine)) {
            if (tsLine.isEmpty())
                break;

            if (tsLine.startsWith("SCLK:")) {
                const QList<QString> data = tsLine.split(u' ', Qt::SkipEmptyParts);
                bool minRes, maxRes;

                if (data.size() < 3)
                    continue;

                sclkMin = data[1].trimmed().remove("mhz", Qt::CaseInsensitive).toInt(&minRes);
                sclkMax = data[2].trimmed().remove("mhz", Qt::CaseInsensitive).toInt(&maxRes);
                sclkValid = minRes && maxRes;
            }
        }

        return PWTS::ROData<PWTS::LNX::AMD::GPUODRanges>({
            {.min = sclkMin, .max = sclkMax}
        }, sclkValid);
    }

    PWTS::RWData<PWTS::LNX::AMD::GPUDPMForcePerfLevel> OSLinux::getAMDGPUDpmForcePerfLevel(const int index) const {
        const QString dpmForcePerfLvl = readSysfs(QString("%1card%2/device/power_dpm_force_performance_level").arg(sysfsDRM).arg(index));
        QString ppod = readSysfs(QString("%1card%2/device/pp_od_clk_voltage").arg(sysfsDRM).arg(index));
        QTextStream ts {&ppod};
        bool validSclk = true;
        PWTS::LNX::AMD::GPUDPMForcePerfLevel data;
        QString tsLine;

        if (dpmForcePerfLvl.isEmpty() || ppod.isEmpty())
            return {};

        data.level = dpmForcePerfLvl;

        while (ts.readLineInto(&tsLine)) {
            tsLine.remove(QChar(u'\0'));

            if (tsLine.startsWith("OD_RANGE") || tsLine.isEmpty())
                break;

            if (tsLine.startsWith("OD_SCLK")) {
                const QString min = ts.readLine();
                const QString max = ts.readLine();
                bool minRes = false;
                bool maxRes = false;

                if (!min.isEmpty())
                    data.sclk.min = min.split(u':').at(1).trimmed().remove("mhz", Qt::CaseInsensitive).toInt(&minRes);

                if (!max.isEmpty())
                    data.sclk.max = max.split(u':').at(1).trimmed().remove("mhz", Qt::CaseInsensitive).toInt(&maxRes);

                validSclk = minRes && maxRes;
            }
        }

        return PWTS::RWData<PWTS::LNX::AMD::GPUDPMForcePerfLevel>(data, validSclk);
    }

    PWTS::RWData<QString> OSLinux::getAMDGPUPowerDpmState(const int index) const {
        const QString dpmState = readSysfs(QString("%1card%2/device/power_dpm_state").arg(sysfsDRM).arg(index));

        return PWTS::RWData<QString>(dpmState, !dpmState.isEmpty());
    }

    bool OSLinux::setAMDGPUDpmForcePerfLevel(const int index, const PWTS::RWData<PWTS::LNX::AMD::GPUDPMForcePerfLevel> &data) const {
        if (!data.isValid())
            return true;

        const PWTS::LNX::AMD::GPUDPMForcePerfLevel dpmData = data.getValue();
        const QString dpmForcePerfLvlPath = QString("%1card%2/device/power_dpm_force_performance_level").arg(sysfsDRM).arg(index);

        if (dpmData.level == "manual") {
            QFile ppodF {QString("%1card%2/device/pp_od_clk_voltage").arg(sysfsDRM).arg(index)};
            QTextStream tsPP {&ppodF};

            if (!ppodF.open(WOTextOpenFlags)) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(QString("failed to open: %1").arg(ppodF.errorString()));

                return false;
            }

            if (!writeSysfs(dpmForcePerfLvlPath, dpmData.level))
                return false;

            if (dpmData.reset) {
                tsPP << 'r';
                tsPP.flush();
            }

            tsPP << QString("s 0 %1").arg(dpmData.sclk.min).toUtf8();
            tsPP.flush();
            tsPP << QString("s 1 %1").arg(dpmData.sclk.max).toUtf8();
            tsPP.flush();
            tsPP << 'c';
            tsPP.flush();

            return tsPP.status() == QTextStream::Ok;
        }

        return writeSysfs(dpmForcePerfLvlPath, dpmData.level);
    }

    bool OSLinux::setAMDGPUPowerDpmState(const int index, const PWTS::RWData<QString> &data) const {
        if (!data.isValid())
            return true;

        return writeSysfs(QString("%1card%2/device/power_dpm_state").arg(sysfsDRM).arg(index), data.getValue());
    }

#ifdef WITH_GPD_FAN
    QString OSLinux::getGPDFanHWMon() const {
        const QDirListing drmIt(sysfsGpdfan, {"hwmon*"}, QDirListing::IteratorFlag::DirsOnly | QDirListing::IteratorFlag::ResolveSymlinks);

        for (const QDirListing::DirEntry &entry: drmIt)
            return entry.fileName();

        return "";
    }

    PWTS::RWData<int> OSLinux::getGPDFanMode(const QString &hwmon) const {
        const QString gpdMode = readSysfs(QString("%1/%2/pwm1_enable").arg(sysfsGpdfan, hwmon));
        bool res;
        const int gpdModeI = gpdMode.toInt(&res);
        const int mode = gpdModeI == 2 ? 0:1;

        return PWTS::RWData<int>(mode, !gpdMode.isEmpty() && res);
    }

    PWTS::ROData<int> OSLinux::getGPDFanSpeed(const QString &hwmon) const {
        const QString speed = readSysfs(QString("%1/%2/fan1_input").arg(sysfsGpdfan, hwmon));
        bool res;
        const int speedI = speed.toInt(&res);
        const int perc = res ? (speedI * 100 / 255) : 0;

        return PWTS::ROData<int>(perc, !speed.isEmpty());
    }

    bool OSLinux::setGPDFanMode(const PWTS::RWData<int> &mode, const QString &hwmon) const {
        if (!mode.isValid())
            return true;

        const int fanMode = mode.getValue();
        const int gpdMode = fanMode == 0 ? 2:1;

        return writeSysfs(QString("%1/%2/pwm1_enable").arg(sysfsGpdfan, hwmon), QString::number(gpdMode));
    }

    bool OSLinux::setGPDFanSpeed(const int speed, const QString &hwmon) const {
        const PWTS::RWData<int> mode = getGPDFanMode(hwmon);
        const int val = 255 * speed / 100;

        if (!mode.isValid() || mode.getValue() == 0)
            return true;

        return writeSysfs(QString("%1/%2/pwm1").arg(sysfsGpdfan, hwmon), QString::number(val));
    }
#endif
}
