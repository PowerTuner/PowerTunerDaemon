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
class MSR_PKG_CST_CONFIG_CONTROL_CU1 final: public MSR_PKG_CST_CONFIG_CONTROL {
public:
    [[nodiscard]]
    PWTS::RWData<PWTS::Intel::PkgCstConfigControl> get(const int cpu) const override {
        uint64_t reg;

        if (!msrDev->read(addr, cpu, reg))
            return {};

        return PWTS::RWData<PWTS::Intel::PkgCstConfigControl>({
            .packageCStateLimit = static_cast<short>(getBitfield(3, 0, reg)),
            .maxCoreCState = static_cast<short>(getBitfield(7, 4, reg)),
            .ioMwaitRedirectionEnable = getBitfield(10, 10, reg) == 1,
            .cfgLock = getBitfield(15, 15, reg) == 1,
            .c3StateAutodemotionEnable = getBitfield(25, 25, reg) == 1,
            .c1StateAutodemotionEnable = getBitfield(26, 26, reg) == 1,
            .c3UndemotionEnable = getBitfield(27, 27, reg) == 1,
            .c1UndemotionEnable = getBitfield(28, 28, reg) == 1,
            .pkgcAutodemotionEnable = getBitfield(29, 29, reg) == 1,
            .pkgcUndemotionEnable = getBitfield(30, 30, reg) == 1,
            .timedMwaitEnable = getBitfield(31, 31, reg) == 1
        }, true);
    }

    [[nodiscard]]
    bool set(const int cpu, const PWTS::RWData<PWTS::Intel::PkgCstConfigControl> &data) const override {
        if (!data.isValid())
            return true;

        const PWTS::Intel::PkgCstConfigControl pkgCstCfgCtrl = data.getValue();
        uint64_t reg;

        if (!msrDev->read(addr, cpu, reg))
            return false;

        reg = setBitfield(3, 0, pkgCstCfgCtrl.packageCStateLimit, reg);
        reg = setBitfield(7, 4, pkgCstCfgCtrl.maxCoreCState, reg);
        reg = setBitfield(10, 10, pkgCstCfgCtrl.ioMwaitRedirectionEnable, reg);
        reg = setBitfield(15, 15, pkgCstCfgCtrl.cfgLock, reg);
        reg = setBitfield(25, 25, pkgCstCfgCtrl.c3StateAutodemotionEnable, reg);
        reg = setBitfield(26, 26, pkgCstCfgCtrl.c1StateAutodemotionEnable, reg);
        reg = setBitfield(27, 27, pkgCstCfgCtrl.c3UndemotionEnable, reg);
        reg = setBitfield(28, 28, pkgCstCfgCtrl.c1UndemotionEnable, reg);
        reg = setBitfield(29, 29, pkgCstCfgCtrl.pkgcAutodemotionEnable, reg);
        reg = setBitfield(30, 30, pkgCstCfgCtrl.pkgcUndemotionEnable, reg);
        reg = setBitfield(31, 31, pkgCstCfgCtrl.timedMwaitEnable, reg);

        if (!msrDev->write(reg, addr, cpu) || !msrDev->read(addr, cpu, reg))
            return false;

        return getBitfield(3, 0, reg) == pkgCstCfgCtrl.packageCStateLimit &&
            getBitfield(7, 4, reg) == pkgCstCfgCtrl.maxCoreCState &&
            getBitfield(10, 10, reg) == pkgCstCfgCtrl.ioMwaitRedirectionEnable &&
            getBitfield(15, 15, reg) == pkgCstCfgCtrl.cfgLock &&
            getBitfield(25, 25, reg) == pkgCstCfgCtrl.c3StateAutodemotionEnable &&
            getBitfield(26, 26, reg) == pkgCstCfgCtrl.c1StateAutodemotionEnable &&
            getBitfield(27, 27, reg) == pkgCstCfgCtrl.c3UndemotionEnable &&
            getBitfield(28, 28, reg) == pkgCstCfgCtrl.c1UndemotionEnable &&
            getBitfield(29, 29, reg) == pkgCstCfgCtrl.pkgcAutodemotionEnable &&
            getBitfield(30, 30, reg) == pkgCstCfgCtrl.pkgcUndemotionEnable &&
            getBitfield(31, 31, reg) == pkgCstCfgCtrl.timedMwaitEnable;
    }
};
}