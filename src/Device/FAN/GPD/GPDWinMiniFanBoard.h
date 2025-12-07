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

#include "../CPUFANDevice.h"

namespace PWTD::GPD {
    class GPDWinMiniFanBoard final: public CPUFANDevice {
    public:
        GPDWinMiniFanBoard(const QSharedPointer<OS> &os, const QString &id): CPUFANDevice(os, id) {
            fanString = "GPD Win Mini Fan Board";
            control = {
                .board = FanBoard::GPD_WIN_MINI,
                .addrPort = 0x4E,
                .dataPort = 0x4F,
                .readAdr = 0x478,
                .writeAdr = {0x47A},
                .modeAdr = {0x47A},
                .maxPWM = 244,
                .controlPath = os->getFanControlPath(FanBoard::GPD_WIN_MINI)
            };
        }
    };
}
