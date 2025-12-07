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

#include "../MCHBAR/MCHBARRegister.h"
#include "../../Utils/CPUUtils.h"
#include "pwtShared/Include/Types/ROData.h"
#include "pwtShared/Include/CPU/Intel/MCHBARRPStateCap.h"

namespace PWTD::Intel {
    class MCHBAR_RP_STATE_CAP final: public MCHBARRegister {
    private:
        struct rpStateCap final {
            uint32_t rp0Capability :8; // 7:0
            // 15:8 reserved:7
            uint32_t pnCapability :8; // 23:16
            // 31:24 reserved:7
        };

        [[nodiscard]]
        bool setBitfields(const uint32_t raw, rpStateCap &regVal) const {
            try {
                regVal.rp0Capability = getBitfield(8, 0, raw);
                regVal.pnCapability = getBitfield(24, 16, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        explicit MCHBAR_RP_STATE_CAP(const uint32_t base): MCHBARRegister(base, 0x5998) {}

        PWTS::ROData<PWTS::Intel::MCHBARRPStateCap> getRPStateCapData() const {
            rpStateCap regVal {};
            uint64_t raw = 0;

            if (!memory->readMem32(raw, addr) || !setBitfields(raw, regVal))
                return {};

            return PWTS::ROData<PWTS::Intel::MCHBARRPStateCap>({
                .rp0 = static_cast<int>(regVal.rp0Capability),
                .pn = static_cast<int>(regVal.pnCapability)
            }, true);
        }
    };
}
