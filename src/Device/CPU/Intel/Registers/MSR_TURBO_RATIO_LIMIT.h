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
#include "pwtShared/Include/CPU/Intel/TurboRatioLimit.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class MSR_TURBO_RATIO_LIMIT final: public CPURegister {
    private:
        struct turboRatioLimit final {
            uint64_t maxRatioLimit1C :8; // 7:0
            uint64_t maxRatioLimit2C :8; // 15:8
            uint64_t maxRatioLimit3C :8; // 23:16
            uint64_t maxRatioLimit4C :8; // 31:24
            uint64_t maxRatioLimit5C :8; // 39:32
            uint64_t maxRatioLimit6C :8; // 47:40
            uint64_t maxRatioLimit7C :8; // 55:48
            uint64_t maxRatioLimit8C :8; // 63:56
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, turboRatioLimit &regVal) const {
            try {
                regVal.maxRatioLimit1C = getBitfield(7, 0, raw);
                regVal.maxRatioLimit2C = getBitfield(15, 8, raw);
                regVal.maxRatioLimit3C = getBitfield(23, 16, raw);
                regVal.maxRatioLimit4C = getBitfield(31, 24, raw);
                regVal.maxRatioLimit5C = getBitfield(39, 32, raw);
                regVal.maxRatioLimit6C = getBitfield(47, 40, raw);
                regVal.maxRatioLimit7C = getBitfield(55, 48, raw);
                regVal.maxRatioLimit8C = getBitfield(63, 56, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const turboRatioLimit &regVal, uint64_t &raw) const {
            try {
                setBitfield(7, 0, regVal.maxRatioLimit1C, raw);
                setBitfield(15, 8, regVal.maxRatioLimit2C, raw);
                setBitfield(23, 16, regVal.maxRatioLimit3C, raw);
                setBitfield(31, 24, regVal.maxRatioLimit4C, raw);
                setBitfield(39, 32, regVal.maxRatioLimit5C, raw);
                setBitfield(47, 40, regVal.maxRatioLimit6C, raw);
                setBitfield(55, 48, regVal.maxRatioLimit7C, raw);
                setBitfield(63, 56, regVal.maxRatioLimit8C, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        MSR_TURBO_RATIO_LIMIT() {
            addr = 0x1ad;
        }

        PWTS::RWData<PWTS::Intel::TurboRatioLimit> getTurboRatioLimitData() const {
            turboRatioLimit regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::TurboRatioLimit>({
                .maxRatioLimit1C = static_cast<int>(regVal.maxRatioLimit1C),
                .maxRatioLimit2C = static_cast<int>(regVal.maxRatioLimit2C),
                .maxRatioLimit3C = static_cast<int>(regVal.maxRatioLimit3C),
                .maxRatioLimit4C = static_cast<int>(regVal.maxRatioLimit4C),
                .maxRatioLimit5C = static_cast<int>(regVal.maxRatioLimit5C),
                .maxRatioLimit6C = static_cast<int>(regVal.maxRatioLimit6C),
                .maxRatioLimit7C = static_cast<int>(regVal.maxRatioLimit7C),
                .maxRatioLimit8C = static_cast<int>(regVal.maxRatioLimit8C)
            }, true);
        }

        [[nodiscard]]
        bool setTurboRatioLimit(const PWTS::RWData<PWTS::Intel::TurboRatioLimit> &data) const {
            if (!data.isValid())
                return true;

            const PWTS::Intel::TurboRatioLimit limit = data.getValue();
            turboRatioLimit regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.maxRatioLimit1C = limit.maxRatioLimit1C;
            regVal.maxRatioLimit2C = limit.maxRatioLimit2C;
            regVal.maxRatioLimit3C = limit.maxRatioLimit3C;
            regVal.maxRatioLimit4C = limit.maxRatioLimit4C;
            regVal.maxRatioLimit5C = limit.maxRatioLimit5C;
            regVal.maxRatioLimit6C = limit.maxRatioLimit6C;
            regVal.maxRatioLimit7C = limit.maxRatioLimit7C;
            regVal.maxRatioLimit8C = limit.maxRatioLimit8C;

            if (!msrUtils->readMSR(raw, addr, 0) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
