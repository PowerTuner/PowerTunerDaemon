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

#include "MSR_POWER_CTL.h"
#include "../../../Utils/CPUUtils.h"

namespace PWTD::Intel {
    class MSR_POWER_CTL_NHLM final: public MSR_POWER_CTL {
    private:
        struct powerCtl final {
            // 0 reserved:1
            uint64_t c1eEnable :1; // 1
            // 63:2 reserved:62
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, powerCtl &regVal) const {
            try {
                regVal.c1eEnable = getBitfield(1, 1, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const powerCtl &regVal, uint64_t &raw) const {
            try {
                setBitfield(1, 1, regVal.c1eEnable, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        PWTS::RWData<PWTS::Intel::PowerCtl> getPowerCtlData() const override {
            powerCtl regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::PowerCtl>({
                .c1eEnable = regVal.c1eEnable == 1,
            }, true);
        }

        [[nodiscard]]
        bool setPowerCtl(const PWTS::RWData<PWTS::Intel::PowerCtl> &data) const override {
            if (!data.isValid())
                return true;

            const PWTS::Intel::PowerCtl powCtl = data.getValue();
            powerCtl regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.c1eEnable = powCtl.c1eEnable;

            if (!msrUtils->readMSR(raw, addr, 0) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
