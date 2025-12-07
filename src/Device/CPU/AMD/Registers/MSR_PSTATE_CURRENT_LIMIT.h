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
#include "pwtShared/Include/CPU/AMD/PStateCurrentLimit.h"
#include "pwtShared/Include/Types/ROData.h"

namespace PWTD::AMD {
    class MSR_PSTATE_CURRENT_LIMIT final: public CPURegister {
    private:
        struct PStateCurrentLimit final {
            uint64_t curPStateLimit :4; // 3:0
            uint64_t pstateMaxVal :4; // 7:4
            // 63:8 reserved:56
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, PStateCurrentLimit &regVal) const {
            try {
                regVal.curPStateLimit = getBitfield(3, 0, raw);
                regVal.pstateMaxVal = getBitfield(7, 4, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        MSR_PSTATE_CURRENT_LIMIT() {
            addr = 0xc0010061;
        }

       PWTS::ROData<PWTS::AMD::PStateCurrentLimit> getPStateCurrentLimitData() const {
            PStateCurrentLimit regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::ROData<PWTS::AMD::PStateCurrentLimit>({
                .curPStateLimit = static_cast<int>(regVal.curPStateLimit),
                .pstateMaxValue = static_cast<int>(regVal.pstateMaxVal)
            }, true);
        }
    };
}
