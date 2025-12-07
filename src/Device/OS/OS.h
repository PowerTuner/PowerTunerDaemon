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

#include "../../Utils/FileLogger/FileLogger.h"
#include "../FAN/Include/FanControls.h"
#include "pwtShared/Include/SystemInfo.h"
#include "pwtShared/Include/Features.h"
#include "pwtShared/Include/GPU/GPUVendor.h"
#include "pwtShared/Include/Packets/DaemonPacket.h"
#include "pwtShared/Include/Packets/ClientPacket.h"

namespace PWTD {
    class OS {
    protected:
        QSharedPointer<PWTS::SystemInfo> sysInfo;
        QSharedPointer<FileLogger> logger;

        [[nodiscard]] virtual QString getBiosVendor() const = 0;
        [[nodiscard]] virtual QString getBiosVersion() const = 0;
        [[nodiscard]] virtual QString getBiosDate() const = 0;
        [[nodiscard]] virtual QString getECVersion() const = 0;
        [[nodiscard]] virtual QString getProductName() const = 0;
        [[nodiscard]] virtual QString getManufacturer() const = 0;
        [[nodiscard]] virtual QString getMicrocodeRevision(int cpu) const = 0;
        [[nodiscard]] virtual int getOnlineCPUCount(int numLogicalCPUs) const = 0;
        [[nodiscard]] virtual quint64 getAvailableRam() const = 0;
        [[nodiscard]] virtual quint64 getSwapSize() const = 0;

    public:
        OS();
        virtual ~OS() = default;

        void collectSystemInfo();
        [[nodiscard]] QSharedPointer<PWTS::SystemInfo> getSystemInfo();
        PWTS::DynamicSystemInfo getDynamicSystemInfo(int numLogicalCPUs) const;

        [[nodiscard]] virtual bool setupOSAccess() const = 0;
        virtual void unsetOSAccess() const = 0;
        [[nodiscard]] virtual QSet<PWTS::Feature> getCPUFeatures(int numLogicalCPUs, PWTS::CPUVendor vendor) const = 0;
        [[nodiscard]] virtual std::pair<PWTS::GPUVendor, QSet<PWTS::Feature>> getGPUFeatures(int index, PWTS::GPUVendor vendor) const = 0;
        [[nodiscard]] virtual bool hasFanControls() const = 0;
        virtual void fillDaemonPacket(const PWTS::Features &features, PWTS::CPUVendor cpuVendor, int numLogicalCPUs, PWTS::DaemonPacket &packet) const = 0;
        [[nodiscard]] virtual QSet<PWTS::DError> applySettings(const PWTS::Features &features, PWTS::CPUVendor cpuVendor, int numLogicalCPUs, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const = 0;
        [[nodiscard]] virtual QList<int> getCPUCoreIndexList() const = 0;
        [[nodiscard]] virtual QList<int> getGPUIndexList() const = 0;
        [[nodiscard]] virtual PWTS::GPUVendor getGPUVendor(int index) const = 0;
        [[nodiscard]] virtual QString getGPUDeviceID(int index) const = 0;
        [[nodiscard]] virtual QString getGPURevisionID(int index) const = 0;
        [[nodiscard]] virtual QString getGPUVBiosVersion(int index) const = 0;
        [[nodiscard]] virtual QString getFanControlPath(FanBoard type) const = 0;
        [[nodiscard]] virtual PWTS::RWData<int> getFanMode(const FanControls &controls) const = 0;
        [[nodiscard]] virtual PWTS::ROData<int> getFanSpeed(const FanControls &controls) const = 0;
        [[nodiscard]] virtual bool setFanMode(const FanControls &controls, const PWTS::RWData<int> &mode) const = 0;
        [[nodiscard]] virtual bool setFanSpeed(const FanControls &controls, int speed) const = 0;
    };
}
