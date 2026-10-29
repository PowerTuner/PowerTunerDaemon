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
#include "../IntelUtils.h"
#include "../../../../external/libPowerTuner/src/Vendor/Intel/Include/HWPRequest.h"

namespace PWTD::Intel {
    class IA32_HWP_REQUEST final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x774;

    public:
        [[nodiscard]]
        std::optional<LIBPWT::Intel::HWPRequest> get(const int cpu) const {
            uint64_t reg;

            if (!msrDev->read(addr, cpu, reg))
                return {};

            return LIBPWT::Intel::HWPRequest {
                .requestPkg = {
                    .min = static_cast<int>(getBitfield(7, 0, reg)),
                    .max = static_cast<int>(getBitfield(15, 8, reg)),
                    .desired = static_cast<int>(getBitfield(23, 16, reg)),
                    .epp = static_cast<int>(getBitfield(31, 24, reg)),
                    .acw = static_cast<int>(static_cast<double>(getBitfield(38, 32, reg)) * std::pow(10, getBitfield(41, 39, reg)))
                },
                .packageControl = getBitfield(42, 42, reg) == 1,
                .acwValid = getBitfield(59, 59, reg) == 1,
                .eppValid = getBitfield(60, 60, reg) == 1,
                .desiredValid = getBitfield(61, 61, reg) == 1,
                .maxValid = getBitfield(62, 62, reg) == 1,
                .minValid = getBitfield(63, 63, reg) == 1
            };
        }

        [[nodiscard]]
        bool set(const int cpu, const LIBPWT::Intel::HWPRequest &data) const {
            const auto [exponent, mantissa] = USecToRawHWPActivityWindow(data.requestPkg.acw);
            uint64_t reg;

            if (!msrDev->read(addr, cpu, reg))
                return false;

            reg = setBitfield(7, 0, data.requestPkg.min, reg);
            reg = setBitfield(15, 8, data.requestPkg.max, reg);
            reg = setBitfield(23, 16, data.requestPkg.desired, reg);
            reg = setBitfield(31, 24, data.requestPkg.epp, reg);
            reg = setBitfield(38, 32, mantissa, reg);
            reg = setBitfield(41, 39, exponent, reg);
            reg = setBitfield(42, 42, data.packageControl, reg);
            reg = setBitfield(59, 59, data.acwValid, reg);
            reg = setBitfield(60, 60, data.eppValid, reg);
            reg = setBitfield(61, 61, data.desiredValid, reg);
            reg = setBitfield(62, 62, data.maxValid, reg);
            reg = setBitfield(63, 63, data.minValid, reg);

            return msrDev->write(reg, addr, cpu) &&
                msrDev->read(addr, cpu, reg) &&
                getBitfield(7, 0, reg) == data.requestPkg.min &&
                getBitfield(15, 8, reg) == data.requestPkg.max &&
                getBitfield(23, 16, reg) == data.requestPkg.desired &&
                getBitfield(31, 24, reg) == data.requestPkg.epp &&
                getBitfield(38, 32, reg) == mantissa &&
                getBitfield(41, 39, reg) == exponent &&
                getBitfield(42, 42, reg) == data.packageControl &&
                getBitfield(59, 59, reg) == data.acwValid &&
                getBitfield(60, 60, reg) == data.eppValid &&
                getBitfield(61, 61, reg) == data.desiredValid &&
                getBitfield(62, 62, reg) == data.maxValid &&
                getBitfield(63, 63, reg) == data.minValid;
        }
    };
}
