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
    class GPDWin4FanBoard final: public CPUFANDevice {
    public:
        GPDWin4FanBoard(const QSharedPointer<OS> &os, const QString &id): CPUFANDevice(os, id) {
            fanString = "GPD Win 4 Fan Board";
            control = {
                .board = FanBoard::GPD_WIN4,
                .addrPort = 0x2E,
                .dataPort = 0x2F,
                .readAdr = 0xC880,
                .writeAdr = {0xC311},
                .modeAdr = {0xC311},
                .maxPWM = 127,
                .controlPath = os->getFanControlPath(FanBoard::GPD_WIN4)
            };
        }
    };
}
