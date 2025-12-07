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

#include <stdexcept>

#include "MSR_PKG_CST_CONFIG_CONTROL.h"
#include "../../../Utils/CPUUtils.h"

namespace PWTD::Intel {
    class MSR_PKG_CST_CONFIG_CONTROL_CU1 final: public MSR_PKG_CST_CONFIG_CONTROL {
    private:
        struct pkgCstConfigControl final {
            uint64_t packageCStateLimit :4; // 3:0
            uint64_t maxCoreCstate :4; // 7:4
            // 9:8 reserved:2
            uint64_t ioMwaitRedirectionEnable :1; // 10
            // 14:11 reserved:4
            uint64_t cfgLock :1; // 15
            // 24:16 reserved:9
            uint64_t c3StateAutodemotionEnable :1; // 25
            uint64_t c1StateAutodemotionEnable :1; // 26
            uint64_t c3UndemotionEnable :1; // 27
            uint64_t c1UndemotionEnable :1; // 28
            uint64_t pkgcAutodemotionEnable :1; // 29
            uint64_t pkgcUndemotionEnable :1; // 30
            uint64_t timedMwaitEnable :1; // 31
            // 63:32 reserved:32
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, pkgCstConfigControl &regVal) const {
            try {
                regVal.packageCStateLimit = getBitfield(3, 0, raw);
                regVal.maxCoreCstate = getBitfield(7, 4, raw);
                regVal.ioMwaitRedirectionEnable = getBitfield(10, 10, raw);
                regVal.cfgLock = getBitfield(15, 15, raw);
                regVal.c3StateAutodemotionEnable = getBitfield(25, 25, raw);
                regVal.c1StateAutodemotionEnable = getBitfield(26, 26, raw);
                regVal.c3UndemotionEnable = getBitfield(27, 27, raw);
                regVal.c1UndemotionEnable = getBitfield(28, 28, raw);
                regVal.pkgcAutodemotionEnable = getBitfield(29, 29, raw);
                regVal.pkgcUndemotionEnable = getBitfield(30, 30, raw);
                regVal.timedMwaitEnable = getBitfield(31, 31, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const pkgCstConfigControl &regVal, uint64_t &raw) const {
            try {
                setBitfield(3, 0, regVal.packageCStateLimit, raw);
                setBitfield(7, 4, regVal.maxCoreCstate, raw);
                setBitfield(10, 10, regVal.ioMwaitRedirectionEnable, raw);
                setBitfield(15, 15, regVal.cfgLock, raw);
                setBitfield(25, 25, regVal.c3StateAutodemotionEnable, raw);
                setBitfield(26, 26, regVal.c1StateAutodemotionEnable, raw);
                setBitfield(27, 27, regVal.c3UndemotionEnable, raw);
                setBitfield(28, 28, regVal.c1UndemotionEnable, raw);
                setBitfield(29, 29, regVal.pkgcAutodemotionEnable, raw);
                setBitfield(30, 30, regVal.pkgcUndemotionEnable, raw);
                setBitfield(31, 31, regVal.timedMwaitEnable, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        PWTS::RWData<PWTS::Intel::PkgCstConfigControl> getPkgCstConfigControlData(const int cpu) const override {
            pkgCstConfigControl regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, cpu) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::PkgCstConfigControl>({
                .packageCStateLimit = static_cast<short>(regVal.packageCStateLimit),
                .maxCoreCState = static_cast<short>(regVal.maxCoreCstate),
                .ioMwaitRedirectionEnable = regVal.ioMwaitRedirectionEnable == 1,
                .cfgLock = regVal.cfgLock == 1,
                .c3StateAutodemotionEnable = regVal.c3StateAutodemotionEnable == 1,
                .c1StateAutodemotionEnable = regVal.c1StateAutodemotionEnable == 1,
                .c3UndemotionEnable = regVal.c3UndemotionEnable == 1,
                .c1UndemotionEnable = regVal.c1UndemotionEnable == 1,
                .pkgcAutodemotionEnable = regVal.pkgcAutodemotionEnable == 1,
                .pkgcUndemotionEnable = regVal.pkgcUndemotionEnable == 1,
                .timedMwaitEnable = regVal.timedMwaitEnable == 1
            }, true);
        }

        [[nodiscard]]
        bool setPkgCstConfigControlData(const int cpu, const PWTS::RWData<PWTS::Intel::PkgCstConfigControl> &data) const override {
            if (!data.isValid())
                return true;

            const PWTS::Intel::PkgCstConfigControl pkgCstCfgCtrl = data.getValue();
            pkgCstConfigControl regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.packageCStateLimit = pkgCstCfgCtrl.packageCStateLimit;
            regVal.maxCoreCstate = pkgCstCfgCtrl.maxCoreCState;
            regVal.ioMwaitRedirectionEnable = pkgCstCfgCtrl.ioMwaitRedirectionEnable;
            regVal.cfgLock = pkgCstCfgCtrl.cfgLock;
            regVal.c3StateAutodemotionEnable = pkgCstCfgCtrl.c3StateAutodemotionEnable;
            regVal.c1StateAutodemotionEnable = pkgCstCfgCtrl.c1StateAutodemotionEnable;
            regVal.c3UndemotionEnable = pkgCstCfgCtrl.c3UndemotionEnable;
            regVal.c1UndemotionEnable = pkgCstCfgCtrl.c1UndemotionEnable;
            regVal.pkgcAutodemotionEnable = pkgCstCfgCtrl.pkgcAutodemotionEnable;
            regVal.pkgcUndemotionEnable = pkgCstCfgCtrl.pkgcUndemotionEnable;
            regVal.timedMwaitEnable = pkgCstCfgCtrl.timedMwaitEnable;

            if (!msrUtils->readMSR(raw, addr, cpu) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, cpu) || !msrUtils->readMSR(cur, addr, cpu))
                return false;

            return cur == raw;
        }
    };
}
