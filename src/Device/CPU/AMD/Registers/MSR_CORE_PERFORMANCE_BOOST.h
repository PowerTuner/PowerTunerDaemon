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

namespace PWTD::AMD {
    class MSR_CORE_PERFORMANCE_BOOST final: public CPURegister {
    private:
        struct corePerformanceBoost final {
            // 24:0 reserved:24
            uint64_t cpbDis :1; // 25
            // 63:26 reserved:38
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, corePerformanceBoost &regVal) const {
            try {
                regVal.cpbDis = getBitfield(25, 25, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const corePerformanceBoost &regVal, uint64_t &raw) const {
            try {
                setBitfield(25, 25, regVal.cpbDis, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        MSR_CORE_PERFORMANCE_BOOST() {
            addr = 0xc0010015;
        }

        PWTS::RWData<int> getCorePerformanceBoostData(const int cpu) const {
            corePerformanceBoost regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, cpu) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<int>(static_cast<int>(regVal.cpbDis), true);
        }

        [[nodiscard]]
        bool setCorePerformanceBoost(const int cpu, const PWTS::RWData<int> &data) const {
            if (!data.isValid())
                return true;

            corePerformanceBoost regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.cpbDis = data.getValue();

            if (!msrUtils->readMSR(raw, addr, cpu) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, cpu) || !msrUtils->readMSR(cur, addr, cpu))
                return false;

            return cur == raw;
        }
    };
}
