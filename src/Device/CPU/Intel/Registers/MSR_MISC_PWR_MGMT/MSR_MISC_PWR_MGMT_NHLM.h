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

#include "MSR_MISC_PWR_MGMT.h"
#include "../../../Utils/CPUUtils.h"

namespace PWTD::Intel {
    class MSR_MISC_PWR_MGMT_NHLM final: public MSR_MISC_PWR_MGMT {
    private:
        struct miscPwrMgmt final {
            uint64_t eistHardwareCoordinationDisable :1; // 0
            uint64_t energyPerformanceBiasEnable :1; // 1 - thread level
            // 63:2 reserved:62
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, miscPwrMgmt &regVal) const {
            try {
                regVal.eistHardwareCoordinationDisable = getBitfield(0, 0, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const miscPwrMgmt &regVal, uint64_t &raw) const {
            try {
                setBitfield(0, 0, regVal.eistHardwareCoordinationDisable, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        PWTS::RWData<PWTS::Intel::MiscPwrMgmt> getMiscPwrMgmtData() const override {
            miscPwrMgmt regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::MiscPwrMgmt>({
                .eistHWCoordinationDisable = regVal.eistHardwareCoordinationDisable == 1,
            }, true);
        }

        [[nodiscard]]
        bool setMiscPwrMgmt(const PWTS::RWData<PWTS::Intel::MiscPwrMgmt> &data) const override {
            if (!data.isValid())
                return true;

            miscPwrMgmt regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.eistHardwareCoordinationDisable = data.getValue().eistHWCoordinationDisable;

            if (!msrUtils->readMSR(raw, addr, 0) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
