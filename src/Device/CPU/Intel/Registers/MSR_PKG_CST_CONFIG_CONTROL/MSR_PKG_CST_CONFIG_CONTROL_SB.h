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
#include "MSR_PKG_CST_CONFIG_CONTROL.h"
#include "../../../Utils/Utils.h"

namespace PWTD::Intel {
    class MSR_PKG_CST_CONFIG_CONTROL_SB final: public MSR_PKG_CST_CONFIG_CONTROL {
    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::PkgCstConfigControl> getPkgCstConfigControlData(const int cpu) const override {
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, cpu))
                return {};

            return PWTS::RWData<PWTS::Intel::PkgCstConfigControl>({
                .packageCStateLimit = static_cast<short>(getBitfield(2, 0, reg)),
                .ioMwaitRedirectionEnable = getBitfield(10, 10, reg) == 1,
                .cfgLock = getBitfield(15, 15, reg) == 1,
                .c3StateAutodemotionEnable = getBitfield(25, 25, reg) == 1,
                .c1StateAutodemotionEnable = getBitfield(26, 26, reg) == 1,
                .c3UndemotionEnable = getBitfield(27, 27, reg) == 1,
                .c1UndemotionEnable = getBitfield(28, 28, reg) == 1
            }, true);
        }

        [[nodiscard]]
        bool setPkgCstConfigControlData(const int cpu, const PWTS::RWData<PWTS::Intel::PkgCstConfigControl> &data) const override {
            if (!data.isValid())
                return true;

            const PWTS::Intel::PkgCstConfigControl pkgCstCfgCtrl = data.getValue();
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, cpu))
                return false;

            reg = setBitfield(2, 0, pkgCstCfgCtrl.packageCStateLimit, reg);
            reg = setBitfield(10, 10, pkgCstCfgCtrl.ioMwaitRedirectionEnable, reg);
            reg = setBitfield(15, 15, pkgCstCfgCtrl.cfgLock, reg);
            reg = setBitfield(25, 25, pkgCstCfgCtrl.c3StateAutodemotionEnable, reg);
            reg = setBitfield(26, 26, pkgCstCfgCtrl.c1StateAutodemotionEnable, reg);
            reg = setBitfield(27, 27, pkgCstCfgCtrl.c3UndemotionEnable, reg);
            reg = setBitfield(28, 28, pkgCstCfgCtrl.c1UndemotionEnable, reg);

            if (!msrUtils->writeMSR(reg, addr, cpu) || !msrUtils->readMSR(reg, addr, cpu))
                return false;

            return getBitfield(2, 0, reg) == pkgCstCfgCtrl.packageCStateLimit &&
                getBitfield(10, 10, reg) == pkgCstCfgCtrl.ioMwaitRedirectionEnable &&
                getBitfield(15, 15, reg) == pkgCstCfgCtrl.cfgLock &&
                getBitfield(25, 25, reg) == pkgCstCfgCtrl.c3StateAutodemotionEnable &&
                getBitfield(26, 26, reg) == pkgCstCfgCtrl.c1StateAutodemotionEnable &&
                getBitfield(27, 27, reg) == pkgCstCfgCtrl.c3UndemotionEnable &&
                getBitfield(28, 28, reg) == pkgCstCfgCtrl.c1UndemotionEnable;
        }
    };
}
