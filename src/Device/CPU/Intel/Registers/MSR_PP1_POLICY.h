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
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class MSR_PP1_POLICY final: public CPURegister {
    private:
        struct PP1Policy final {
            uint64_t priority :5; // 4:0
            // 63:5 reserved:59
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, PP1Policy &regVal) const {
            try {
                regVal.priority = getBitfield(4, 0, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const PP1Policy &regVal, uint64_t &raw) const {
            try {
                setBitfield(4, 0, regVal.priority, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        MSR_PP1_POLICY() {
            addr = 0x642;
        }

        PWTS::RWData<int> getPP1Priority() const {
            PP1Policy regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<int>(static_cast<int>(regVal.priority), true);
        }

        [[nodiscard]]
        bool setPP1Priority(const PWTS::RWData<int> &data) const {
            if (!data.isValid())
                return true;

            PP1Policy regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.priority = data.getValue();

            if (!msrUtils->readMSR(raw, addr, 0) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
