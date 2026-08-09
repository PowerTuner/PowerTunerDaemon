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
#include "pwtShared/Include/CPU/Intel/HWPRequestPkg.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class IA32_HWP_REQUEST_PKG final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x772;

    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::HWPRequestPkg> get() const {
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, 0))
                return {};

            return PWTS::RWData<PWTS::Intel::HWPRequestPkg>({
                .min = static_cast<int>(getBitfield(7, 0, reg)),
                .max = static_cast<int>(getBitfield(15, 8, reg)),
                .desired = static_cast<int>(getBitfield(23, 16, reg)),
                .epp = static_cast<int>(getBitfield(31, 24, reg)),
                .acw = static_cast<int>(static_cast<double>(getBitfield(38, 32, reg)) * std::pow(10, getBitfield(41, 39, reg)))
            }, true);
        }

        [[nodiscard]]
        bool set(const PWTS::RWData<PWTS::Intel::HWPRequestPkg> &data) const {
            if (!data.isValid() || data.isIgnored())
                return true;

            const PWTS::Intel::HWPRequestPkg hwpReq = data.getValue();
            const auto [exponent, mantissa] = USecToRawHWPActivityWindow(hwpReq.acw);
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, 0))
                return false;

            reg = setBitfield(7, 0, hwpReq.min, reg);
            reg = setBitfield(15, 8, hwpReq.max, reg);
            reg = setBitfield(23, 16, hwpReq.desired, reg);
            reg = setBitfield(31, 24, hwpReq.epp, reg);
            reg = setBitfield(38, 32, mantissa, reg);
            reg = setBitfield(41, 39, exponent, reg);

            if (!msrUtils->writeMSR(reg, addr, 0) || !msrUtils->readMSR(reg, addr, 0))
                return false;

            return getBitfield(7, 0, reg) == hwpReq.min &&
                getBitfield(15, 8, reg) == hwpReq.max &&
                getBitfield(23, 16, reg) == hwpReq.desired &&
                getBitfield(31, 24, reg) == hwpReq.epp &&
                getBitfield(38, 32, reg) == mantissa &&
                getBitfield(41, 39, reg) == exponent;
        }
    };
}