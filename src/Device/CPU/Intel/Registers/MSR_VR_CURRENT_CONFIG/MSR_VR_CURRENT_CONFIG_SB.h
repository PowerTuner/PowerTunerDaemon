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

#include "MSR_VR_CURRENT_CONFIG.h"
#include "../../../Utils/CPUUtils.h"

namespace PWTD::Intel {
    class MSR_VR_CURRENT_CONFIG_SB final: public MSR_VR_CURRENT_CONFIG {
    private:
        struct vrCurrentConfig final {
            uint64_t pl4 :13; // 12:0
            // 30:13 reserved:18
            uint64_t lock :1; // 31
            // 63:32 reserved:32
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, vrCurrentConfig &regVal) const {
            try {
                regVal.pl4 = getBitfield(12, 0, raw);
                regVal.lock = getBitfield(31, 31, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

        [[nodiscard]]
        bool setRawValue(const vrCurrentConfig &regVal, uint64_t &raw) const {
            try {
                setBitfield(12, 0, regVal.pl4, raw);
                setBitfield(31, 31, regVal.lock, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        PWTS::RWData<PWTS::Intel::VRCurrentConfig> getVrCurrentConfigData() const override {
            vrCurrentConfig regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::RWData<PWTS::Intel::VRCurrentConfig>({
                .pl4 = static_cast<int>(regVal.pl4 * 0.125 * 1000),
                .lock = regVal.lock == 1
            }, true);
        }

        [[nodiscard]]
        bool setVrCurrentConfig(const PWTS::RWData<PWTS::Intel::VRCurrentConfig> &data) const override {
            if (!data.isValid())
                return true;

            const PWTS::Intel::VRCurrentConfig vrCfg = data.getValue();
            vrCurrentConfig regVal {};
            uint64_t raw = 0, cur = 0;

            regVal.pl4 = static_cast<int>(vrCfg.pl4 / 0.125 / 1000);
            regVal.lock = vrCfg.lock;

            if (!msrUtils->readMSR(raw, addr, 0) || !setRawValue(regVal, raw) || !msrUtils->writeMSR(raw, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return cur == raw;
        }
    };
}
