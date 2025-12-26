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
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class IA32_PM_ENABLE final: public CPURegister {
    private:
        struct ia32PmEnable final {
            uint64_t hwpEnable :1; // 0
            // 63:1 reserved:63
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, ia32PmEnable &regVal) const {
            try {
                regVal.hwpEnable = getBitfield(0, 0, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const ia32PmEnable &regVal, uint64_t &raw) const {
            try {
                setBitfield(0, 0, regVal.hwpEnable, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        IA32_PM_ENABLE() {
            addr = 0x770;
        }

        PWTS::RWData<int> getHWPEnableBit() const {
            ia32PmEnable regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<int>(static_cast<int>(regVal.hwpEnable), true);
        }

        [[nodiscard]]
        bool setHWPEnableBit(const PWTS::RWData<int> &data) const {
            if (!data.isValid() || data.isIgnored())
                return true;

            ia32PmEnable regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.hwpEnable = data.getValue();

            if (!msrUtils->readMSR(raw, addr, 0) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
