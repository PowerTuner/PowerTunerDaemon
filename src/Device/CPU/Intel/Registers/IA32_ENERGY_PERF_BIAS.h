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
    class IA32_ENERGY_PERF_BIAS final: public CPURegister {
    private:
        struct ia32EnergyPerfBias final {
            uint64_t powerPolicyPreference :4; // 3:0
            // 63:4 reserved:60
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, ia32EnergyPerfBias &regVal) const {
            try {
                regVal.powerPolicyPreference = getBitfield(3, 0, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const ia32EnergyPerfBias &regVal, uint64_t &raw) const {
            try {
                setBitfield(3, 0, regVal.powerPolicyPreference, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        IA32_ENERGY_PERF_BIAS() {
            addr = 0x1b0;
        }

        PWTS::RWData<int> getPowerPolicyPreference() const {
            ia32EnergyPerfBias regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<int>(static_cast<int>(regVal.powerPolicyPreference), true);
        }

        [[nodiscard]]
        bool setPowerPolicyPreference(const PWTS::RWData<int> &data) const {
            if (!data.isValid())
                return true;

            ia32EnergyPerfBias regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.powerPolicyPreference = data.getValue();

            if (!msrUtils->readMSR(raw, addr, 0) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
