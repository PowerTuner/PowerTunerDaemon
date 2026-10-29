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
#include "../../../../external/libPowerTuner/src/Vendor/Intel/Include/HWPRequestPkg.h"

namespace PWTD::Intel {
    class IA32_HWP_REQUEST_PKG final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x772;

    public:
        [[nodiscard]]
        std::optional<LIBPWT::Intel::HWPRequestPkg> get() const {
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return {};

            return LIBPWT::Intel::HWPRequestPkg {
                .min = static_cast<int>(getBitfield(7, 0, reg)),
                .max = static_cast<int>(getBitfield(15, 8, reg)),
                .desired = static_cast<int>(getBitfield(23, 16, reg)),
                .epp = static_cast<int>(getBitfield(31, 24, reg)),
                .acw = static_cast<int>(static_cast<double>(getBitfield(38, 32, reg)) * std::pow(10, getBitfield(41, 39, reg)))
            };
        }

        [[nodiscard]]
        bool set(const LIBPWT::Intel::HWPRequestPkg &data) const {
            const auto [exponent, mantissa] = USecToRawHWPActivityWindow(data.acw);
            uint64_t reg;

            if (!msrDev->read(addr, 0, reg))
                return false;

            reg = setBitfield(7, 0, data.min, reg);
            reg = setBitfield(15, 8, data.max, reg);
            reg = setBitfield(23, 16, data.desired, reg);
            reg = setBitfield(31, 24, data.epp, reg);
            reg = setBitfield(38, 32, mantissa, reg);
            reg = setBitfield(41, 39, exponent, reg);

            return msrDev->write(reg, addr, 0) &&
                msrDev->read(addr, 0, reg) &&
                getBitfield(7, 0, reg) == data.min &&
                getBitfield(15, 8, reg) == data.max &&
                getBitfield(23, 16, reg) == data.desired &&
                getBitfield(31, 24, reg) == data.epp &&
                getBitfield(38, 32, reg) == mantissa &&
                getBitfield(41, 39, reg) == exponent;
        }
    };
}