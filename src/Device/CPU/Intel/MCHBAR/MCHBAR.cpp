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
#include "MCHBAR.h"
#include "MCHBARUtils.h"
#include "../../Utils/Memory/MemoryFactory.h"
#include "../Include/CPUFamily.h"
#include "../Include/CPUModel.h"
#include "../Include/ModelRegistersIncludes.h"

namespace PWTD::Intel {
    MCHBAR::MCHBAR(const int cpuFamily) {
        baseAddress = getMCHBARBaseAddress(cpuFamily);
    }

    bool MCHBAR::init(const int cpuFamily, const int cpuModel) {
        if (baseAddress == 0 || !MemoryFactory::getInstance()->isAccessible())
            return false;

        switch (cpuFamily) {
            case Family6: {
                switch (cpuModel) {
                    case IvyBridge:
                    case IceLakeU: {
                        mchbarPackageRaplLimit.reset(new MCHBAR_PACKAGE_RAPL_LIMIT_IVB(baseAddress));
                    }
                        break;
                    case TigerLakeU:
                    case LunarLake: {
                        mchbarPackageRaplLimit.reset(new MCHBAR_PACKAGE_RAPL_LIMIT_TGL(baseAddress));
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

        cacheStaticRegistersData();
        return true;
    }

    void MCHBAR::cacheStaticRegistersData() const {
        regsCache.reset(new RegistersCache);

        if (!mchbarPackageRaplLimit.isNull()) {
            const std::unique_ptr<MCHBAR_PACKAGE_POWER_SKU_UNIT> powUnit = std::make_unique<MCHBAR_PACKAGE_POWER_SKU_UNIT>(baseAddress);

            regsCache->pkgPowerSkuUnit = powUnit->getPkgSKUPowerUnitData();
        }
    }

    QSet<PWTS::Feature> MCHBAR::getFeatures() const {
        QSet<PWTS::Feature> features;

        if (!mchbarPackageRaplLimit.isNull()) {
            features.unite({PWTS::Feature::INTEL_MCHBAR_PKG_RAPL_LIMIT, PWTS::Feature::INTEL_MCHBAR_GROUP});

            if (dynamic_cast<MCHBAR_PACKAGE_RAPL_LIMIT_IVB *>(mchbarPackageRaplLimit.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_MCHBAR_PKG_RAPL_LIMIT_IVB);
            else if (dynamic_cast<MCHBAR_PACKAGE_RAPL_LIMIT_TGL *>(mchbarPackageRaplLimit.get()) != nullptr)
                features.insert(PWTS::Feature::INTEL_MCHBAR_PKG_RAPL_LIMIT_TGL);
        }

        return features;
    }

    void MCHBAR::fillPacketData(const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::INTEL_MCHBAR_GROUP))
            return;

        if (features.contains(PWTS::Feature::INTEL_MCHBAR_PKG_RAPL_LIMIT))
            packet.intelData->mchbarPkgRaplLimit = mchbarPackageRaplLimit->getPkgRaplLimitData(regsCache->pkgPowerSkuUnit);
    }

    QSet<PWTS::DError> MCHBAR::applySettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet) const {
        if (!features.contains(PWTS::Feature::INTEL_MCHBAR_GROUP))
            return {};

        const QSharedPointer<PWTS::Intel::IntelData> data = packet.intelData;
        QSet<PWTS::DError> errors;

        if (features.contains(PWTS::Feature::INTEL_MCHBAR_PKG_RAPL_LIMIT) && !mchbarPackageRaplLimit->setPkgRaplLimit(data->mchbarPkgRaplLimit, regsCache->pkgPowerSkuUnit))
            errors.insert(PWTS::DError::W_POWER_LIMIT_MCHBAR);

        return errors;
    }
}
