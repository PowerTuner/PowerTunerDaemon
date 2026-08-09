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
#include "../Utils/IntelRegisterUtils.h"
#include "pwtShared/Include/CPU/Intel/HWPRequest.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class IA32_HWP_REQUEST final: public CPURegister {
    private:
        static constexpr unsigned addr = 0x774;

    public:
        [[nodiscard]]
        PWTS::RWData<PWTS::Intel::HWPRequest> getHWPRequestData(const int cpu) const {
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, cpu))
                return {};

            return PWTS::RWData<PWTS::Intel::HWPRequest>({
                .requestPkg = {
                    .min = static_cast<int>(getBitfield(7, 0, reg)),
                    .max = static_cast<int>(getBitfield(15, 8, reg)),
                    .desired = static_cast<int>(getBitfield(23, 16, reg)),
                    .epp = static_cast<int>(getBitfield(31, 24, reg)),
                    .acw = static_cast<int>(getBitfield(38, 32, reg) * std::pow(10, getBitfield(41, 39, reg)))
                },
                .packageControl = getBitfield(42, 42, reg) == 1,
                .acwValid = getBitfield(59, 59, reg) == 1,
                .eppValid = getBitfield(60, 60, reg) == 1,
                .desiredValid = getBitfield(61, 61, reg) == 1,
                .maxValid = getBitfield(62, 62, reg) == 1,
                .minValid = getBitfield(63, 63, reg) == 1
            }, true);
        }

        [[nodiscard]]
        bool setHWPRequest(const int cpu, const PWTS::RWData<PWTS::Intel::HWPRequest> &data) const {
            if (!data.isValid() || data.isIgnored())
                return true;

            const PWTS::Intel::HWPRequest hwpReq = data.getValue();
            const HWPActivityWindowBits acwBits = getHWPActivityWindowBitsFromMicroSecond(hwpReq.requestPkg.acw);
            uint64_t reg;

            if (!msrUtils->readMSR(reg, addr, cpu))
                return false;

            reg = setBitfield(7, 0, hwpReq.requestPkg.min, reg);
            reg = setBitfield(15, 8, hwpReq.requestPkg.max, reg);
            reg = setBitfield(23, 16, hwpReq.requestPkg.desired, reg);
            reg = setBitfield(31, 24, hwpReq.requestPkg.epp, reg);
            reg = setBitfield(38, 32, acwBits.mantissa, reg);
            reg = setBitfield(41, 39, acwBits.exponent, reg);
            reg = setBitfield(42, 42, hwpReq.packageControl, reg);
            reg = setBitfield(59, 59, hwpReq.acwValid, reg);
            reg = setBitfield(60, 60, hwpReq.eppValid, reg);
            reg = setBitfield(61, 61, hwpReq.desiredValid, reg);
            reg = setBitfield(62, 62, hwpReq.maxValid, reg);
            reg = setBitfield(63, 63, hwpReq.minValid, reg);

            if (!msrUtils->writeMSR(reg, addr, cpu) || !msrUtils->readMSR(reg, addr, cpu))
                return false;

            return getBitfield(7, 0, reg) == hwpReq.requestPkg.min &&
                getBitfield(15, 8, reg) == hwpReq.requestPkg.max &&
                getBitfield(23, 16, reg) == hwpReq.requestPkg.desired &&
                getBitfield(31, 24, reg) == hwpReq.requestPkg.epp &&
                getBitfield(38, 32, reg) == acwBits.mantissa &&
                getBitfield(41, 39, reg) == acwBits.exponent &&
                (getBitfield(42, 42, reg) == 1) == hwpReq.packageControl &&
                (getBitfield(59, 59, reg) == 1) == hwpReq.acwValid &&
                (getBitfield(60, 60, reg) == 1) == hwpReq.eppValid &&
                (getBitfield(61, 61, reg) == 1) == hwpReq.desiredValid &&
                (getBitfield(62, 62, reg) == 1) == hwpReq.maxValid &&
                (getBitfield(63, 63, reg) == 1) == hwpReq.minValid;
        }
    };
}
