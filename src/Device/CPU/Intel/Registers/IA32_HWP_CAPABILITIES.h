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
#include "pwtShared/Include/CPU/Intel/HWPCapabilities.h"
#include "pwtShared/Include/Types/ROData.h"

namespace PWTD::Intel {
    class IA32_HWP_CAPABILITIES final: public CPURegister {
    private:
        struct ia32HWPCapabilities final {
            uint64_t highestPerformance :8; // 7:0
            uint64_t guaranteedPerformance :8; // 15:8
            uint64_t mostEfficientPerformance :8; // 23:16
            uint64_t lowestPerformance :8; // 31:24
            // 63:32 reserved:32
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, ia32HWPCapabilities &regVal) const {
            try {
                regVal.highestPerformance = getBitfield(7, 0, raw);
                regVal.guaranteedPerformance = getBitfield(15, 8, raw);
                regVal.mostEfficientPerformance = getBitfield(23, 16, raw);
                regVal.lowestPerformance = getBitfield(31, 24, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        IA32_HWP_CAPABILITIES() {
            addr = 0x771;
        }

        PWTS::ROData<PWTS::Intel::HWPCapabilities> getHWPCapabilitiesData(const int cpu) const {
            ia32HWPCapabilities regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, cpu) || !setBitfields(raw, regVal))
                return {};

            return PWTS::ROData<PWTS::Intel::HWPCapabilities>({
                .lowestPerf = static_cast<int>(regVal.lowestPerformance),
                .highestPerf = static_cast<int>(regVal.highestPerformance)
            }, true);
        }
    };
}
