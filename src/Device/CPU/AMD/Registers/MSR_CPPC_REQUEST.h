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
#include "../../../../external/libPowerTuner/src/Vendor/AMD/Include/CPPCRequest.h"

namespace PWTD::AMD {
    class MSR_CPPC_REQUEST final: public CPURegister {
    private:
        static constexpr uint32_t addr = 0xc00102b3;

    public:
        [[nodiscard]]
        std::optional<LIBPWT::AMD::CPPCRequest> get(const int cpu) const {
            uint64_t reg;

            if (!msrDev->read(addr, cpu, reg))
                return {};

            return LIBPWT::AMD::CPPCRequest {
                .maxPerf = static_cast<int>(getBitfield(7, 0, reg)),
                .minPerf = static_cast<int>(getBitfield(15, 8, reg)),
                .desiredPerf = static_cast<int>(getBitfield(23, 16, reg)),
                .epp = static_cast<int>(getBitfield(31, 24, reg))
            };
        }

        [[nodiscard]]
        bool set(const int cpu, const LIBPWT::AMD::CPPCRequest &data) const {
            uint64_t reg;

            if (!msrDev->read(addr, cpu, reg))
                return false;

            reg = setBitfield(7, 0, data.maxPerf, reg);
            reg = setBitfield(15, 8, data.minPerf, reg);
            reg = setBitfield(23, 16, data.desiredPerf, reg);
            reg = setBitfield(31, 24, data.epp, reg);

            return msrDev->write(reg, addr, cpu) &&
                msrDev->read(addr, cpu, reg) &&
                getBitfield(7, 0, reg) == data.maxPerf &&
                getBitfield(15, 8, reg) == data.minPerf &&
                getBitfield(23, 16, reg) && data.desiredPerf &&
                getBitfield(31, 24, reg) && data.epp;
        }
    };
}
