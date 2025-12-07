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

#include "../../CPURegister.h"
#include "../../Utils/CPUUtils.h"
#include "pwtShared/Include/CPU/Intel/TurboPowerCurrentLimit.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class MSR_TURBO_POWER_CURRENT_LIMIT final: public CPURegister {
    private:
        struct turboPowerCurrentLimit final {
            uint64_t tdpLimit :15; // 14:0
            uint64_t tdpLimitOverrideEnable :1; // 15
            uint64_t tdcLimit :15; // 30:16
            uint64_t tdcLimitOverrideEnable :1; // 31
            // 63:32 reserved:32
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, turboPowerCurrentLimit &regVal) const {
            try {
                regVal.tdpLimit = getBitfield(14, 0, raw);
                regVal.tdpLimitOverrideEnable = getBitfield(15, 15, raw);
                regVal.tdcLimit = getBitfield(30, 16, raw);
                regVal.tdcLimitOverrideEnable = getBitfield(31, 31, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const turboPowerCurrentLimit &regVal, uint64_t &raw) const {
            try {
                setBitfield(14, 0, regVal.tdpLimit, raw);
                setBitfield(15, 15, regVal.tdpLimitOverrideEnable, raw);
                setBitfield(30, 16, regVal.tdcLimit, raw);
                setBitfield(31, 31, regVal.tdcLimitOverrideEnable, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        MSR_TURBO_POWER_CURRENT_LIMIT() {
            addr = 0x1ac;
        }

        PWTS::RWData<PWTS::Intel::TurboPowerCurrentLimit> getTurboPowerCurrentLimitData() const {
            turboPowerCurrentLimit regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::TurboPowerCurrentLimit>({
                .tdpLimit = static_cast<int>(regVal.tdpLimit * 0.125 * 1000),
                .tdpLimitOverride = regVal.tdpLimitOverrideEnable == 1,
                .tdcLimit = static_cast<int>(regVal.tdcLimit * 0.125 * 1000),
                .tdcLimitOverride = regVal.tdcLimitOverrideEnable == 1
            }, true);
        }

        [[nodiscard]]
        bool setTurboPowerCurrentLimit(const PWTS::RWData<PWTS::Intel::TurboPowerCurrentLimit> &data) const {
            if (!data.isValid())
                return true;

            const PWTS::Intel::TurboPowerCurrentLimit limit = data.getValue();
            turboPowerCurrentLimit regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.tdpLimit = static_cast<uint64_t>(limit.tdpLimit / 0.125 / 1000);
            regVal.tdpLimitOverrideEnable = limit.tdpLimitOverride;
            regVal.tdcLimit = static_cast<uint64_t>(limit.tdcLimit / 0.125 / 1000);
            regVal.tdcLimitOverrideEnable = limit.tdcLimitOverride;

            if (!msrUtils->readMSR(raw, addr, 0) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
