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

#include "libryzenadj/ryzenadj.h"
#include "../../Utils/FileLogger/FileLogger.h"
#include "pwtShared/Include/Feature.h"
#include "pwtShared/Include/Packets/DaemonPacket.h"
#include "pwtShared/Include/Packets/ClientPacket.h"

namespace PWTD::AMD {
    class RyzenAdj final {
    private:
        static constexpr uint32_t curveOptimizerBase = 0x100000;
        QSharedPointer<FileLogger> logger;
        int cpuCoreCount = 0;
        int ryStaticGfxClock = -1;
        int ryMinGfxClock = -1;
        int ryMaxGfxClock = -1;
        int ryPowerProfile = -1;
        int ryCOAll = 0;
        QList<int> ryCOCore;

        [[nodiscard]] bool ryzenAdjSet(ADJ_OPT opt, uint32_t value) const;
        [[nodiscard]] bool ryzenAdjSet(ADJ_OPT opt, const PWTS::RWData<int> &data) const;
        [[nodiscard]] bool ryzenAdjSet(ADJ_OPT opt, const PWTS::RWData<uint32_t> &data) const;
        PWTS::RWData<int> ryzenAdjGet(ADJ_OPT opt, int valueMult) const;
        PWTS::ROData<int> ryzenAdjRead(ADJ_OPT opt, int valueMult) const;
        [[nodiscard]] bool refreshRyzenAdjTable() const;
        void fillPackageData(const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const;
        void fillCoreData(int cpu, const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const;
        void applyPackageSettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors);
        void applyCoreSettings(int cpu, int coreIdx, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors);
        [[nodiscard]] bool setStaticGfxClock(const PWTS::RWData<int> &data);
        [[nodiscard]] bool setMinGfxClock(const PWTS::RWData<int> &data);
        [[nodiscard]] bool setMaxGfxClock(const PWTS::RWData<int> &data);
        [[nodiscard]] bool setPowerProfile(const PWTS::RWData<int> &data);
        [[nodiscard]] bool setCurveOptimizerAll(const PWTS::RWData<int> &data);
        [[nodiscard]] bool setCurveOptimizerCore(int cpu, const PWTS::RWData<int> &data);

    public:
        RyzenAdj();
        ~RyzenAdj();

        [[nodiscard]] bool init(int numCores);
        [[nodiscard]] QSet<PWTS::Feature> getFeatures();
        void fillPacketData(const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const;
        [[nodiscard]] QSet<PWTS::DError> applySettings(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet);
        [[nodiscard]] PWTS::ROData<int> getTemperature() const;
    };
}
