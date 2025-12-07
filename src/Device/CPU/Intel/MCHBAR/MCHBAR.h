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

#include "../Include/RegistersIncludes.h"
#include "pwtShared/Include/Feature.h"
#include "pwtShared/Include/Packets/DaemonPacket.h"
#include "pwtShared/Include/Packets/ClientPacket.h"

namespace PWTD::Intel {
    class MCHBAR final {
    private:
        struct RegistersCache final {
            PWTS::ROData<MCHBAR_PACKAGE_POWER_SKU_UNIT::PkgPowerSKUUnits> pkgPowerSkuUnit;
        };

        uint32_t baseAddress;
        mutable QScopedPointer<RegistersCache> regsCache;
        QScopedPointer<MCHBAR_PACKAGE_RAPL_LIMIT> mchbarPackageRaplLimit;

        void cacheStaticRegistersData() const;

    public:
        explicit MCHBAR(int cpuFamily);

        [[nodiscard]] bool init(int cpuFamily, int cpuModel);
        [[nodiscard]] QSet<PWTS::Feature> getFeatures() const;
        void fillPacketData(const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const;
        [[nodiscard]] QSet<PWTS::DError> applySettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet) const;
    };
}
