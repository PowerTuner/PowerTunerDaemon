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

#include <QFile>
#include <QRegularExpression>

#include "../OS.h"

namespace PWTD::LNX {
    class OSLinux final: public OS {
    private:
        const QFlags<QIODevice::OpenModeFlag> ROTextOpenFlags = QFile::ReadOnly | QFile::Text;
        const QFlags<QIODevice::OpenModeFlag> WOTextOpenFlags = QFile::WriteOnly | QFile::Text;
        const QRegularExpression gpuCardFolderRex {R"(^card([0-9]+)$)"};
        const QRegularExpression miscPmDevPathRex {R"(^\/sys\/bus\/(pci|usb)\/devices\/[\/\-\:\.\w\d]+\/power\/control$)"};
        const QRegularExpression miscPmBlockPathRex {R"(^\/sys\/block\/[\w\d]+\/device\/power\/control$)"};
        static constexpr char sysfsCPU[] = R"(/sys/devices/system/cpu/)";
        static constexpr char sysfsDMI[] = R"(/sys/class/dmi/id/)";
        static constexpr char sysfsDRM[] = R"(/sys/class/drm/)";
        static constexpr char sysfsBlock[] = R"(/sys/class/block/)";
        static constexpr char sysfsCPUIdle[] = R"(/sys/devices/system/cpu/cpuidle/)";
        static constexpr char sysfsSMT[] = R"(/sys/devices/system/cpu/smt/control)";
#ifdef WITH_GPD_FAN
        static constexpr char sysfsGpdfan[] = R"(/sys/devices/platform/gpd_fan/hwmon)";
#endif

        void fillIntelGPUData(int index, const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const;
        void fillAMDGPUData(int index, const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const;
        void fillPackageData(const PWTS::Features &features, const PWTS::DaemonPacket &packet) const;
        void fillThreadData(int cpu, const PWTS::Features &features, const PWTS::DaemonPacket &packet) const;
        void applyIntelGPUSettings(int index, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;
        void applyAMDGPUSettings(int index, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;
        void applyPackageSettings(const PWTS::Features &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;
        void applyThreadSettings(int cpu, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;
        [[nodiscard]] QString readSysfs(const QString &path, bool existsErrorLog = true) const;
        [[nodiscard]] bool writeSysfs(const QString &path, const QString &value) const;
        [[nodiscard]] bool hasSMT() const;
        PWTS::ROData<bool> hasCPULogicalOffFeature(int cpu) const;
        [[nodiscard]] bool deviceHasRuntimePM(const QString &path) const;
        PWTS::ROData<PWTS::LNX::CPUFrequencyLimits> getCPUFrequencyLimits(int cpu) const;
        PWTS::RWData<PWTS::MinMax> getCPUFrequency(int cpu) const;
        PWTS::RWData<QString> getSMT() const;
        PWTS::ROData<int> getCoreID(int cpu) const;
        PWTS::RWData<int> getCPUOnlineStatus(int cpu) const;
        PWTS::ROData<PWTS::LNX::CPUScalingAvailableGovernors> getCPUScalingAvailableGovernors(int cpu) const;
        PWTS::RWData<QString> getCPUScalingGovernor(int cpu) const;
        PWTS::ROData<QList<QString>> getAvailableCPUIdleGovernors() const;
        PWTS::RWData<QString> getCPUIdleGovernor() const;
        [[nodiscard]] QMap<QString, PWTS::LNX::BlockDeviceQueSched> getBlockDevices() const;
        [[nodiscard]] QList<PWTS::LNX::MiscPMDevice> getMiscPMI2cDevices() const;
        [[nodiscard]] QList<PWTS::LNX::MiscPMDevice> getMiscPMPCIDevices() const;
        [[nodiscard]] QList<PWTS::LNX::MiscPMDevice> getMiscPMBlockDevices() const;
        [[nodiscard]] QList<PWTS::LNX::MiscPMDevice> getMiscPMUSBDevices() const;
        [[nodiscard]] QList<PWTS::LNX::MiscPMDevice> getMiscPMDevices() const;
        [[nodiscard]] bool setCPUFrequency(int cpu, const PWTS::RWData<PWTS::MinMax> &data) const;
        [[nodiscard]] bool setSMT(const PWTS::RWData<QString> &state) const;
        [[nodiscard]] bool setCPUOnlineStatus(int cpu, const PWTS::RWData<int> &data) const;
        [[nodiscard]] bool setCPUScalingGovernor(int cpu, const PWTS::RWData<QString> &data) const;
        [[nodiscard]] bool setCPUIdleGovernor(const PWTS::RWData<QString> &data) const;
        [[nodiscard]] bool setBlockDevices(const QMap<QString, PWTS::LNX::BlockDeviceQueSched> &blockDevList) const;
        [[nodiscard]] bool setMiscPMDevices(const QList<PWTS::LNX::MiscPMDevice> &miscPMDevList) const;

#ifdef WITH_AMD
        [[nodiscard]] QSet<PWTS::Feature> getAMDCPUFeatures() const;
        void fillAMDPackageData(const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const;
        void fillAMDThreadData(int cpu, const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const;
        void fillAMDDaemonPacket(const QSet<PWTS::Feature> &features, int numLogicalCPUs, PWTS::DaemonPacket &packet) const;
        void applyAMDPackageSettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;
        void applyAMDThreadSettings(int cpu, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;
        void applyAMDSettings(const QSet<PWTS::Feature> &features, int numLogicalCPUs, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;
        PWTS::ROData<PWTS::LNX::AMD::AMDPStateData> getAMDPStateData(int cpu) const;
        PWTS::RWData<QString> getAMDPStateStatus() const;
        PWTS::RWData<QString> getAMDPStateEPPPreference(int cpu) const;
        [[nodiscard]] bool setAMDPStateStatus(const PWTS::RWData<QString> &data) const;
        [[nodiscard]] bool setAMDPStateEPPPreference(int cpu, const PWTS::RWData<QString> &data) const;
#endif
#ifdef WITH_GPD_FAN
        [[nodiscard]] QString getGPDFanHWMon() const;
        [[nodiscard]] PWTS::RWData<int> getGPDFanMode(const QString &hwmon) const;
        [[nodiscard]] PWTS::ROData<int> getGPDFanSpeed(const QString &hwmon) const;
        [[nodiscard]] bool setGPDFanMode(const PWTS::RWData<int> &mode, const QString &hwmon) const;
        [[nodiscard]] bool setGPDFanSpeed(int speed, const QString &hwmon) const;
#endif

        // intel gpu
        [[nodiscard]] bool hasIntelGPURPSFreq(int index) const;
        [[nodiscard]] bool hasIntelGPUBoost(int index) const;
        [[nodiscard]] QSet<PWTS::Feature> getIntelGPUFeatures(int index) const;
        PWTS::ROData<PWTS::LNX::Intel::GPURPSLimits> getIntelGPURPSLimits(int index) const;
        PWTS::RWData<PWTS::MinMax> getIntelGPUFrequency(int index) const;
        PWTS::RWData<int> getIntelGPUBoost(int index) const;
        [[nodiscard]] bool setIntelGPUFrequency(int index, const PWTS::RWData<PWTS::MinMax> &data) const;
        [[nodiscard]] bool setIntelGPUBoost(int index, const PWTS::RWData<int> &data) const;

        // amd gpu
        [[nodiscard]] bool hasAMDGPUDpmForcePerfLevel(int index) const;
        [[nodiscard]] bool hasAMDGPUPowerDpmState(int index) const;
        [[nodiscard]] QSet<PWTS::Feature> getAMDGPUFeatures(int index) const;
        PWTS::ROData<PWTS::LNX::AMD::GPUODRanges> getAMDGPUODRanges(int index) const;
        PWTS::RWData<PWTS::LNX::AMD::GPUDPMForcePerfLevel> getAMDGPUDpmForcePerfLevel(int index) const;
        PWTS::RWData<QString> getAMDGPUPowerDpmState(int index) const;
        [[nodiscard]] bool setAMDGPUDpmForcePerfLevel(int index, const PWTS::RWData<PWTS::LNX::AMD::GPUDPMForcePerfLevel> &data) const;
        [[nodiscard]] bool setAMDGPUPowerDpmState(int index, const PWTS::RWData<QString> &data) const;

    protected:
        [[nodiscard]] QString getBiosVendor() const override;
        [[nodiscard]] QString getBiosVersion() const override;
        [[nodiscard]] QString getBiosDate() const override;
        [[nodiscard]] QString getECVersion() const override;
        [[nodiscard]] QString getProductName() const override;
        [[nodiscard]] QString getManufacturer() const override;
        [[nodiscard]] QString getMicrocodeRevision(int cpu) const override;
        [[nodiscard]] int getOnlineCPUCount(int numLogicalCPUs) const override;
        [[nodiscard]] quint64 getAvailableRam() const override;
        [[nodiscard]] quint64 getSwapSize() const override;

    public:
        [[nodiscard]] bool setupOSAccess() const override;
        void unsetOSAccess() const override;
        [[nodiscard]] QSet<PWTS::Feature> getCPUFeatures(int numLogicalCPUs, PWTS::CPUVendor vendor) const override;
        [[nodiscard]] std::pair<PWTS::GPUVendor, QSet<PWTS::Feature>> getGPUFeatures(int index, PWTS::GPUVendor vendor) const override;
        [[nodiscard]] bool hasFanControls() const override;
        void fillDaemonPacket(const PWTS::Features &features, PWTS::CPUVendor cpuVendor, int numLogicalCPUs, PWTS::DaemonPacket &packet) const override;
        [[nodiscard]] QSet<PWTS::DError> applySettings(const PWTS::Features &features, PWTS::CPUVendor cpuVendor, int numLogicalCPUs, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const override;
        [[nodiscard]] QList<int> getCPUCoreIndexList() const override;
        [[nodiscard]] QList<int> getGPUIndexList() const override;
        [[nodiscard]] PWTS::GPUVendor getGPUVendor(int index) const override;
        [[nodiscard]] QString getGPUDeviceID(int index) const override;
        [[nodiscard]] QString getGPURevisionID(int index) const override;
        [[nodiscard]] QString getGPUVBiosVersion(int index) const override;
        [[nodiscard]] QString getFanControlPath(FanBoard type) const override;
        [[nodiscard]] PWTS::RWData<int> getFanMode(const FanControls &controls) const override;
        [[nodiscard]] PWTS::ROData<int> getFanSpeed(const FanControls &controls) const override;
        [[nodiscard]] bool setFanMode(const FanControls &controls, const PWTS::RWData<int> &mode) const override;
        [[nodiscard]] bool setFanSpeed(const FanControls &controls, int speed) const override;
    };
}
