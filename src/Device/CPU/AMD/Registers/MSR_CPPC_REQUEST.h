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
#include "pwtShared/Include/CPU/AMD/CPPCRequest.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::AMD {
    class MSR_CPPC_REQUEST final: public CPURegister {
    private:
        struct cppcRequest final {
            uint64_t maxPerf :8; // 7:0
            uint64_t minPerf :8; // 15:8
            uint64_t desPerf :8; // 23:16
            uint64_t epp :8; // 31:24
            // 63:32 reserved:32
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, cppcRequest &regVal) const {
            try {
                regVal.maxPerf = getBitfield(7, 0, raw);
                regVal.minPerf = getBitfield(15, 8, raw);
                regVal.desPerf = getBitfield(23, 16, raw);
                regVal.epp = getBitfield(31, 24, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const cppcRequest &regVal, uint64_t &raw) const {
            try {
                setBitfield(7, 0, regVal.maxPerf, raw);
                setBitfield(15, 8, regVal.minPerf, raw);
                setBitfield(23, 16, regVal.desPerf, raw);
                setBitfield(31, 24, regVal.epp, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        MSR_CPPC_REQUEST() {
            addr = 0xc00102b3;
        }

        PWTS::RWData<PWTS::AMD::CPPCRequest> getCPPCRequestData(const int cpu) const {
            cppcRequest regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, cpu) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::AMD::CPPCRequest>({
                .maxPerf = static_cast<int>(regVal.maxPerf),
                .minPerf = static_cast<int>(regVal.minPerf),
                .desPerf = static_cast<int>(regVal.desPerf),
                .epp = static_cast<int>(regVal.epp)
            }, true);
        }

        [[nodiscard]]
        bool setCPPCRequest(const int cpu, const PWTS::RWData<PWTS::AMD::CPPCRequest> &data) const {
            if (!data.isValid())
                return true;

            const PWTS::AMD::CPPCRequest cppcReq = data.getValue();
            cppcRequest regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.maxPerf = cppcReq.maxPerf;
            regVal.minPerf = cppcReq.minPerf;
            regVal.desPerf = cppcReq.desPerf;
            regVal.epp = cppcReq.epp;

            if (!msrUtils->readMSR(raw, addr, cpu) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, cpu) || !msrUtils->readMSR(cur, addr, cpu))
                return false;

            return cur == raw;
        }
    };
}
