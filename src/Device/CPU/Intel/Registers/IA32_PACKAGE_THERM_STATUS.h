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
#include "pwtShared/Include/CPU/Intel/PkgThermalStatusInfo.h"
#include "pwtShared/Include/Types/ROData.h"

namespace PWTD::Intel {
    class IA32_PACKAGE_THERM_STATUS final: public CPURegister {
    private:
        struct ia32PkgThermStatus final {
            uint64_t pkgThermalStatus :1; // 0
            uint64_t pkgThermalStatusLog :1; // 1
            uint64_t pkgProchotEvent :1; // 2
            uint64_t pkgProchotLog :1; // 3
            uint64_t pkgCriticalTempStatus :1; // 4
            uint64_t pkgCriticalTempStatusLog :1; // 5
            uint64_t pkgThermalThreshold1Status :1; // 6
            uint64_t pkgThermalThreshold1StatusLog :1; // 7
            uint64_t pkgThermalThreshold2Status :1; // 8
            uint64_t pkgThermalThreshold2StatusLog :1; // 9
            uint64_t pkgPowerLimitationStatus :1; // 10
            uint64_t pkgPowerLimitationLog :1; // 11
            // 15:12 reserved:4
            uint64_t pkgDigitalReadout :7; // 22:16
            // 25:23 reserved:3
            uint64_t hardwareFeedbakInterfaceStructureChangeStatus :1; // 26:26
            // 63:27 reserved:37
        };

        [[nodiscard]]
        bool setBitfields(const uint64_t raw, ia32PkgThermStatus &regVal) const {
            try {
                regVal.pkgDigitalReadout = getBitfield(22, 16, raw);

            } catch ([[maybe_unused]] std::invalid_argument const &e) {
                if (logger->isLevel(PWTS::LogLevel::Error))
                    logger->write(e.what());

                return false;
            }

            return true;
        }

    public:
        IA32_PACKAGE_THERM_STATUS() {
            addr = 0x1b1;
        }

        PWTS::ROData<PWTS::Intel::PkgThermalStatusInfo> getPkgThermStatusData() const {
            ia32PkgThermStatus regVal {};
            uint64_t raw = 0;

            if (!msrUtils->readMSR(raw, addr, 0) || !setBitfields(raw, regVal))
                return {};

            return PWTS::ROData<PWTS::Intel::PkgThermalStatusInfo>({
                .digitalReadout = static_cast<int>(regVal.pkgDigitalReadout)
            }, true);
        }
    };
}
