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
#include "../../Utils/Memory/Factory.h"
#include "../Include/CPUFamily.h"
#include "../Include/CPUModel.h"
#include "../Include/ModelRegistersIncludes.h"
#include "../IntelUtils.h"

namespace PWTD::Intel {
    MCHBAR::MCHBAR(const int cpuFamily, const int cpuExtModel, LIBPWT::DeviceData &devData) {
        baseAddress = getMCHBARBaseAddress(cpuExtModel);

        if (baseAddress == 0 || MEM::factory()->forbidden())
            return;

        initialized = true;

        setupCPUModelRegisters(cpuFamily, cpuExtModel);
        setFeatures(devData);
    }

    void MCHBAR::setupCPUModelRegisters(const int family, const int extModel) {
        switch (family) {
            case Family6: {
                switch (extModel) {
                    case IvyBridge:
                    case IceLakeU: {
                        mchbarPackageRaplLimit = std::make_unique<MCHBAR_PACKAGE_RAPL_LIMIT_IVB>(baseAddress);
                    }
                        break;
                    case TigerLakeU:
                    case LunarLake: {
                        mchbarPackageRaplLimit = std::make_unique<MCHBAR_PACKAGE_RAPL_LIMIT_TGL>(baseAddress);
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
    }

    void MCHBAR::setFeatures(LIBPWT::DeviceData &devData) {
        std::unordered_set<LIBPWT::Feature> &features = devData.daemonInfo->features.cpu;

        if (mchbarPackageRaplLimit) {
            const std::optional<MCHBAR_PACKAGE_POWER_SKU_UNIT::Units> pu = MCHBAR_PACKAGE_POWER_SKU_UNIT(baseAddress).get();

            if (pu) {
                features.insert(LIBPWT::Feature::INTEL_MCHBAR_PKG_RAPL_LIMIT);
                powerSkuUnit = *pu;

                if (dynamic_cast<MCHBAR_PACKAGE_RAPL_LIMIT_IVB *>(mchbarPackageRaplLimit.get()) != nullptr)
                    features.insert(LIBPWT::Feature::INTEL_MCHBAR_PKG_RAPL_LIMIT_IVB);
                else if (dynamic_cast<MCHBAR_PACKAGE_RAPL_LIMIT_TGL *>(mchbarPackageRaplLimit.get()) != nullptr)
                    features.insert(LIBPWT::Feature::INTEL_MCHBAR_PKG_RAPL_LIMIT_TGL);
            } else {
                mchbarPackageRaplLimit.reset();
            }
        }
    }

    void MCHBAR::read(LIBPWT::DeviceData &devData) const {
        if (mchbarPackageRaplLimit) {
            devData.intelMsr->mchbarPkgRaplLimit = mchbarPackageRaplLimit->get(powerSkuUnit);

            if (!devData.intelMsr->mchbarPkgRaplLimit)
                devData.errors.push_back(LIBPWT::Error::R_POWER_LIMIT_MCHBAR);
        }
    }

    std::vector<LIBPWT::Error> MCHBAR::write(const LIBPWT::DeviceData &clientData) const {
        std::vector<LIBPWT::Error> errors;

        if (mchbarPackageRaplLimit && clientData.intelMsr->mchbarPkgRaplLimit && !mchbarPackageRaplLimit->set(*clientData.intelMsr->mchbarPkgRaplLimit, powerSkuUnit))
            errors.push_back(LIBPWT::Error::W_POWER_LIMIT_MCHBAR);

        return errors;
    }
}
