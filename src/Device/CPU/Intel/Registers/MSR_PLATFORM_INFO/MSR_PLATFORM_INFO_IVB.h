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

#include "MSR_PLATFORM_INFO.h"
#include "../../../Utils/CPUUtils.h"

namespace PWTD::Intel {
    class MSR_PLATFORM_INFO_IVB final: public MSR_PLATFORM_INFO {
    private:
        struct platformInfoReg final {
            // 7:0 reserved:8
            uint64_t maxNonTurboRatio :8; // 15:8
            // 27:16 reserved:12
            uint64_t programmableRatioLimitForTurboMode :1; // 28
            uint64_t programmableTDPLimitForTurboMode :1; // 29
            // 31:30 reserved:2
            uint64_t lowPowerModeSupport :1; // 32
            uint64_t numberOfConfigTDPLevels :2; // 34:33
            // 39:35 reserved:5
            uint64_t maxEfficiencyRatio :8; // 47:40
            uint64_t minOperatingRatio :8; // 55:48
            // 63:56 reserved:8
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, platformInfoReg &regVal) const {
            try {
                regVal.maxNonTurboRatio = getBitfield(15, 8, raw);
                regVal.programmableRatioLimitForTurboMode = getBitfield(28, 28, raw);
                regVal.programmableTDPLimitForTurboMode = getBitfield(29, 29, raw);
                regVal.lowPowerModeSupport = getBitfield(32, 32, raw);
                regVal.numberOfConfigTDPLevels = getBitfield(34, 33, raw);
                regVal.maxEfficiencyRatio = getBitfield(47, 40, raw);
                regVal.minOperatingRatio = getBitfield(55, 48, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        PWTS::ROData<PlatformInfoData> getPlatformInfoData() const override {
            platformInfoReg regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::ROData<PlatformInfoData>({
                .programmableRatioLimitForTurboMode = regVal.programmableRatioLimitForTurboMode == 1,
                .programmableTDPLimitForTurboMode = regVal.programmableTDPLimitForTurboMode == 1,
            }, true);
        }
    };
}
