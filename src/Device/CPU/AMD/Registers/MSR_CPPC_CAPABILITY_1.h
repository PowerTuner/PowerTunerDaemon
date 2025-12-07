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
#include "pwtShared/Include/CPU/AMD/CPPCCapability1.h"
#include "pwtShared/Include/Types/ROData.h"

namespace PWTD::AMD {
    class MSR_CPPC_CAPABILITY_1 final: public CPURegister {
    private:
        struct cppcCapability1 final {
            uint64_t lowestPerf :8; // 7:0
            uint64_t lowNonLinPerf :8; // 15:8
            uint64_t nominalPerf :8; // 23:16
            uint64_t highestPerf :8; // 31:24
            // 63:32 reserved:32
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, cppcCapability1 &regVal) const {
            try {
                regVal.lowestPerf = getBitfield(7, 0, raw);
                regVal.lowNonLinPerf = getBitfield(15, 8, raw);
                regVal.nominalPerf = getBitfield(23, 16, raw);
                regVal.highestPerf = getBitfield(31, 24, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        MSR_CPPC_CAPABILITY_1() {
            addr = 0xc00102b0;
        }

        PWTS::ROData<PWTS::AMD::CPPCCapability1> getCPPCCapability1Data(const int cpu) const {
            cppcCapability1 regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, cpu) || !setBitfields(raw, regVal))
                return {};

            return PWTS::ROData<PWTS::AMD::CPPCCapability1>({
                .lowestPerf = static_cast<int>(regVal.lowestPerf),
                .lowNonLinPerf = static_cast<int>(regVal.lowNonLinPerf),
                .nominalPerf = static_cast<int>(regVal.nominalPerf),
                .highestPerf = static_cast<int>(regVal.highestPerf)
            }, true);
        }
    };
}
