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
#include "Includes/RegistersInlcudes.h"
#include "SMU/RyzenAdj.h"

namespace PWTD::AMD {
    class AMDCPU final: public CPUDevice {
    private:
        QScopedPointer<RyzenAdj> ryzenAdj;
        QScopedPointer<MSR_PSTATE_CURRENT_LIMIT> msrPStateCurrentLimit;
        QScopedPointer<MSR_PSTATE_CONTROL> msrPStateControl;
        QScopedPointer<MSR_CORE_PERFORMANCE_BOOST> msrCorePerformanceBoost;
        QScopedPointer<MSR_CPPC_CAPABILITY_1> msrCppcCapability1;
        QScopedPointer<MSR_CPPC_ENABLE> msrCppcEnable;
        QScopedPointer<MSR_CPPC_REQUEST> msrCppcRequest;

        [[nodiscard]] bool hasCorePerformanceBoostBit() const;
        [[nodiscard]] bool hasHWPStateBit() const;
        [[nodiscard]] bool hasCPPCBit() const;
        void fillPackageData(const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const;
        void fillCoreData(int cpu, const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const;
        void fillThreadData(int cpu, const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const;
        void applyPackageSettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;
        void applyThreadSettings(int cpu, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;

    public:
        AMDCPU(const QSharedPointer<cpu_id_t> &cpuID, const QSharedPointer<cpu_raw_data_t> &cpuRawData);

        [[nodiscard]] QSet<PWTS::Feature> getFeatures() const override;
        void fillDaemonPacket(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, PWTS::DaemonPacket &packet) const override;
        [[nodiscard]] QSet<PWTS::DError> applySettings(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const override;
        [[nodiscard]] PWTS::ROData<int> getTemperature() const override;
    };
}
