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
#include "../../CPURegister.h"
#include "../../../Utils/Utils.h"
#include "pwtShared/Include/CPU/AMD/CPPCRequest.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::AMD {
    class MSR_CPPC_REQUEST final: public CPURegister {
    private:
        static constexpr uint32_t addr = 0xc00102b3;

    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::AMD::CPPCRequest> getCPPCRequestData(const int cpu) const {
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, cpu))
                return {};

            return PWTS::RWData<PWTS::AMD::CPPCRequest>({
                .maxPerf = static_cast<int>(getBitfield(7, 0, reg)),
                .minPerf = static_cast<int>(getBitfield(15, 8, reg)),
                .desPerf = static_cast<int>(getBitfield(23, 16, reg)),
                .epp = static_cast<int>(getBitfield(31, 24, reg))
            }, true);
        }

        [[nodiscard]]
        bool setCPPCRequest(const int cpu, const PWTS::RWData<PWTS::AMD::CPPCRequest> &data) const {
            if (!data.isValid() || data.isIgnored())
                return true;

            const PWTS::AMD::CPPCRequest cppcReq = data.getValue();
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, cpu))
                return false;

            reg = setBitfield(7, 0, cppcReq.maxPerf, reg);
            reg = setBitfield(15, 8, cppcReq.minPerf, reg);
            reg = setBitfield(23, 16, cppcReq.desPerf, reg);
            reg = setBitfield(31, 24, cppcReq.epp, reg);

            if (!msrUtils->writeMSR(reg, addr, cpu) || !msrUtils->readMSR(reg, addr, cpu))
                return false;

            return getBitfield(7, 0, reg) == cppcReq.maxPerf &&
                getBitfield(15, 8, reg) == cppcReq.minPerf &&
                getBitfield(23, 16, reg) && cppcReq.desPerf &&
                getBitfield(31, 24, reg) && cppcReq.epp;
        }
    };
}
