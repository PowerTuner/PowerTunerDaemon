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

#include "libcpuid.h"
#include "pwtShared/Include/CPU/CpuInfo.h"
#include "pwtShared/Include/Feature.h"
#include "pwtShared/Include/Packets/DaemonPacket.h"
#include "pwtShared/Include/Packets/ClientPacket.h"
#include "../../Utils/FileLogger/FileLogger.h"
#include "Utils/MSR/MSR.h"

namespace PWTD {
    class CPUDevice {
    protected:
        QSharedPointer<FileLogger> logger;
        QSharedPointer<PWTS::CpuInfo> cpuInfo;
        QSharedPointer<cpu_raw_data_t> cpuidRaw;
        QSharedPointer<MSR> msrDev;

    public:
        CPUDevice(const QSharedPointer<cpu_id_t> &cpuid, const QSharedPointer<cpu_raw_data_t> &cpuRawData);
        virtual ~CPUDevice() = default;

        [[nodiscard]] virtual QSet<PWTS::Feature> getFeatures() const = 0;
        virtual void fillDaemonPacket(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, PWTS::DaemonPacket &packet) const = 0;
        [[nodiscard]] virtual QSet<PWTS::DError> applySettings(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const = 0;
        [[nodiscard]] virtual PWTS::ROData<int> getTemperature() const = 0;

        [[nodiscard]] QSharedPointer<PWTS::CpuInfo> getCpuInfo() const { return cpuInfo; }
    };
}
